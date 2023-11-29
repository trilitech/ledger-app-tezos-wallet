# Copyright 2023 Functori <contact@functori.com>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from enum import IntEnum
from typing import Union

from ragger.backend.interface import BackendInterface, RAPDU
from ragger.bip import pack_derivation_path
from ragger.error import ExceptionRAPDU

class CLA(IntEnum):
    DEFAULT = 0x80

class INS(IntEnum):
    VERSION                   = 0x00
    AUTHORIZE_BAKING          = 0x01
    GET_PUBLIC_KEY            = 0x02
    PROMPT_PUBLIC_KEY         = 0x03
    SIGN                      = 0x04
    SIGN_UNSAFE               = 0x05
    RESET                     = 0x06
    QUERY_AUTH_KEY            = 0x07
    QUERY_MAIN_HWM            = 0x08
    GIT                       = 0x09
    SETUP                     = 0x0a
    QUERY_ALL_HWM             = 0x0b
    DEAUTHORIZE               = 0x0c
    QUERY_AUTH_KEY_WITH_CURVE = 0x0d
    HMAC                      = 0x0e
    SIGN_WITH_HASH            = 0x0f

class INDEX(IntEnum):
    FIRST      = 0x00
    OTHER      = 0x01
    LAST       = 0x80
    OTHER_LAST = 0x81

class SIGNATURE_TYPE(IntEnum):
    ED25519       = 0x00
    SECP256K1     = 0x01
    SECP256R1     = 0x02
    BIP32_ED25519 = 0x03

class StatusCode(IntEnum):
    SECURITY                  = 0x6982
    HID_REQUIRED              = 0x6983
    REJECT                    = 0x6985
    WRONG_VALUES              = 0x6A80
    REFERENCED_DATA_NOT_FOUND = 0x6A88
    WRONG_PARAM               = 0x6B00
    WRONG_LENGTH              = 0x6C00
    INVALID_INS               = 0x6D00
    CLASS                     = 0x6E00
    OK                        = 0x9000
    UNEXPECTED_STATE          = 0x9001
    UNEXPECTED_SIGN_STATE     = 0x9002
    UNKNOWN_CX_ERR            = 0x9003
    UNKNOWN                   = 0x90FF
    WRONG_LENGTH_FOR_INS      = 0x917E
    MEMORY_ERROR              = 0x9200
    PARSE_ERROR               = 0x9405

class APP_KIND(IntEnum):
    WALLET = 0x00
    BAKING = 0x01

class Account:
    def __init__(self,
                 path: Union[str, bytes],
                 sig_type: SIGNATURE_TYPE,
                 public_key: str):
        self.path: bytes = \
            pack_derivation_path(path) if isinstance(path, str) \
            else path
        self.sig_type: SIGNATURE_TYPE = sig_type
        self.public_key: str = public_key

MAX_APDU_SIZE: int = 235

class TezosBackend(BackendInterface):

    def _exchange(self,
                  ins: INS,
                  index: INDEX = INDEX.FIRST,
                  sig_type: Union[SIGNATURE_TYPE, None] = None,
                  payload: bytes = b'') -> bytes:

        assert len(payload) <= MAX_APDU_SIZE, f"Apdu too large"

        # Set to a non-existent value to ensure that p2 is unused
        p2: int = sig_type if sig_type is not None else 0xff

        rapdu: RAPDU = self.exchange(CLA.DEFAULT,
                                     ins,
                                     p1=index,
                                     p2=p2,
                                     data=payload)

        if rapdu.status != StatusCode.OK:
            raise ExceptionRAPDU(rapdu.status, rapdu.data)

        return rapdu.data

    def git(self) -> bytes:
        return self._exchange(INS.GIT)

    def version(self) -> bytes:
        return self._exchange(INS.VERSION)

    def get_public_key(self,
                       account: Account,
                       with_prompt: bool = False) -> bytes:

        ins = INS.PROMPT_PUBLIC_KEY if with_prompt else INS.GET_PUBLIC_KEY

        return self._exchange(ins,
                              sig_type=account.sig_type,
                              payload=account.path)

    def _ask_sign(self, ins: INS, account: Account) -> None:
        data: bytes = self._exchange(ins, sig_type=account.sig_type, payload=account.path)
        assert not data

    def _continue_sign(self, ins: INS, message: Union[str, bytes], last: bool) -> bytes:
        if isinstance(message, str): message = bytes.fromhex(message)
        index: INDEX = INDEX.OTHER
        if last: index = INDEX(index | INDEX.LAST)
        return self._exchange(ins, index, payload=message)

    def sign(self,
             account: Account,
             message: Union[str, bytes],
             with_hash: bool = False,
             apdu_size: int = MAX_APDU_SIZE) -> bytes:
        if isinstance(message, str): message = bytes.fromhex(message)
        assert message, "Do not sign empty message"

        ins = INS.SIGN_WITH_HASH if with_hash else INS.SIGN

        self._ask_sign(ins, account)

        while(message):
            payload = message[:apdu_size]
            message = message[apdu_size:]
            last = not message
            data = self._continue_sign(ins, payload, last)
            if last:
                return data
            else:
                assert not data

        assert False, "We should have already returned"
