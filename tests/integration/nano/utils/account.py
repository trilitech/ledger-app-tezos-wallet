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

import base58
from enum import IntEnum
from ragger.bip import pack_derivation_path
from typing import Union

class SIGNATURE_TYPE(IntEnum):
    ED25519       = 0x00
    SECP256K1     = 0x01
    SECP256R1     = 0x02
    BIP32_ED25519 = 0x03

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

    def __repr__(self) -> str:
        return self.public_key

    @property
    def base58_decoded(self) -> bytes:

        # Get the public_key without prefix
        public_key = base58.b58decode_check(self.public_key)

        if self.sig_type in [
                SIGNATURE_TYPE.ED25519,
                SIGNATURE_TYPE.BIP32_ED25519
        ]:
            prefix = bytes.fromhex("0d0f25d9") # edpk(54)
        elif self.sig_type == SIGNATURE_TYPE.SECP256K1:
            prefix = bytes.fromhex("03fee256") # sppk(55)
        elif self.sig_type == SIGNATURE_TYPE.SECP256R1:
            prefix = bytes.fromhex("03b28b7f") # p2pk(55)
        else:
            assert False, \
                "Account do not have a right signature type: {account.sig_type}"
        assert public_key.startswith(prefix), \
            "Expected prefix {prefix.hex()} but got {public_key.hex()}"

        public_key = public_key[len(prefix):]

        if self.sig_type in [
                SIGNATURE_TYPE.SECP256K1,
                SIGNATURE_TYPE.SECP256R1
        ]:
            assert public_key[0] in [0x02, 0x03], \
                "Expected a prefix kind of 0x02 or 0x03 but got {public_key[0]}"
            public_key = public_key[1:]

        return public_key

    def check_public_key(self, data: bytes) -> None:

        # `data` should be:
        # length + kind + pk
        # kind : 02=odd, 03=even, 04=uncompressed
        # pk length = 32 for compressed, 64 for uncompressed
        assert len(data) == data[0] + 1
        if data[1] == 0x04: # public key uncompressed
            assert data[0] == 1 + 32 + 32
        elif data[1] in [0x02, 0x03]: # public key even or odd (compressed)
            assert data[0] == 1 + 32
        else:
            assert False, \
                "Expected a prefix kind of 0x02, 0x03 or 0x04 but got {data[1]}"
        data = data[2:2+32]

        public_key = self.base58_decoded
        assert data == public_key, \
            f"Expected public key {public_key.hex()} but got {data.hex()}"
