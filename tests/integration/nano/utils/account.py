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
