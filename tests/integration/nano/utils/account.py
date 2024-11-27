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

"""Module providing an account interface."""

from enum import IntEnum
from typing import Union

import base58
from pytezos import pytezos
from pytezos.crypto.encoding import base58_encode
from pytezos.crypto.key import Key as PytezosKey
from ragger.bip import pack_derivation_path

from .message import Message

class SigType(IntEnum):
    """Class representing signature type."""

    ED25519       = 0x00
    SECP256K1     = 0x01
    SECP256R1     = 0x02
    BIP32_ED25519 = 0x03

    def __str__(self) -> str:
        return self.name

class Signature(bytes):
    """Class representing signature."""

    def __str__(self) -> str:
        return self.decode()

    @staticmethod
    def from_secp256_tlv(tlv: Union[bytes, bytearray]) -> bytes:
        """Get the signature encapsulated in a TLV."""
        # See:
        # https://developers.ledger.com/docs/embedded-app/crypto-api/lcx__ecdsa_8h/#cx_ecdsa_sign
        # TLV: 30 || L || 02 || Lr || r || 02 || Ls || s
        if isinstance(tlv, bytes):
            tlv = bytearray(tlv)
        header_tag_index = 0
        # Remove the unwanted parity information set here.
        tlv[header_tag_index] &= ~0x01
        if tlv[header_tag_index] != 0x30:
            raise ValueError("Invalid TLV tag")
        len_index = 1
        if tlv[len_index] != len(tlv) - 2:
            raise ValueError("Invalid TLV length")
        first_tag_index = 2
        if tlv[first_tag_index] != 0x02:
            raise ValueError("Invalid TLV tag")
        r_len_index = 3
        r_index = 4
        r_len = tlv[r_len_index]
        second_tag_index = r_index + r_len
        if tlv[second_tag_index] != 0x02:
            raise ValueError("Invalid TLV tag")
        s_len_index = second_tag_index + 1
        s_index = s_len_index + 1
        s_len = tlv[s_len_index]
        r = tlv[r_index : r_index + r_len]
        s = tlv[s_index : s_index + s_len]
        # Sometimes \x00 are added or removed
        # A size adjustment is required here.
        def adjust_size(data, size):
            return data[-size:].rjust(size, b'\x00')
        return adjust_size(r, 32) + adjust_size(s, 32)

    @classmethod
    def from_bytes(cls, data: bytes, sig_type: SigType) -> 'Signature':
        """Get the signature according to the SigType."""
        if sig_type in {SigType.ED25519, SigType.BIP32_ED25519}:
            prefix = b'edsig'
        elif sig_type == SigType.SECP256K1:
            prefix = b'spsig'
            data = Signature.from_secp256_tlv(data)
        elif sig_type == SigType.SECP256R1:
            prefix = b'p2sig'
            data = Signature.from_secp256_tlv(data)
        else:
            assert False, f"Wrong signature type: {sig_type}"

        return cls(base58_encode(data, prefix))


class Account:
    """Class representing account."""

    path: bytes
    sig_type: Union[SigType, int]
    __key: str

    def __init__(self,
                 path: Union[str, bytes],
                 sig_type: Union[SigType, int],
                 key: str):
        self.path = \
            pack_derivation_path(path) if isinstance(path, str) \
            else path
        self.sig_type = sig_type
        self.__key = key

    def __repr__(self) -> str:
        return self.__key

    @property
    def base58_decoded(self) -> bytes:
        """Decodes public_key from base58 encoding."""

        # Get the public_key without prefix
        public_key = base58.b58decode_check(self.key.public_key())

        if self.sig_type in [
                SigType.ED25519,
                SigType.BIP32_ED25519
        ]:
            prefix = bytes.fromhex("0d0f25d9") # edpk(54)
        elif self.sig_type == SigType.SECP256K1:
            prefix = bytes.fromhex("03fee256") # sppk(55)
        elif self.sig_type == SigType.SECP256R1:
            prefix = bytes.fromhex("03b28b7f") # p2pk(55)
        else:
            assert False, \
                "Account do not have a right signature type: {account.sig_type}"
        assert public_key.startswith(prefix), \
            "Expected prefix {prefix.hex()} but got {public_key.hex()}"

        public_key = public_key[len(prefix):]

        if self.sig_type in [
                SigType.SECP256K1,
                SigType.SECP256R1
        ]:
            assert public_key[0] in [0x02, 0x03], \
                "Expected a prefix kind of 0x02 or 0x03 but got {public_key[0]}"
            public_key = public_key[1:]

        return public_key

    def check_public_key(self, data: bytes) -> None:
        """Checks if the data correspond to the public_key."""

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

    @property
    def key(self) -> PytezosKey:
        """pytezos key of the account."""
        return pytezos.using(key=self.__key).key

    def check_signature(
            self,
            data: bytes,
            message: Message,
            with_hash: bool):
        """Checks if signature correspond to a signature of message sign by the account."""
        if with_hash:
            assert data.startswith(message.hash), \
                f"Expected a starting hash {message.hash.hex()} but got {data.hex()}"
            data = data[len(message.hash):]

        signature = Signature.from_bytes(data, SigType(self.sig_type))

        assert self.key.verify(signature, bytes(message)), \
            f"Fail to verify signature {signature!r}, \n\
            with account {self} \n\
            and message {message}"
