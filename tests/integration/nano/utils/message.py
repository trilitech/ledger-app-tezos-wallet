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

"""Implemenation of sent messages."""

from hashlib import blake2b
from typing import Union

class Message:
    """Class representing messages."""

    HASH_SIZE = 32

    value: bytes

    def __init__(self, value: bytes):
        self.value = value

    @classmethod
    def from_bytes(cls, value: Union[str, bytes]) -> 'Message':
        """Get message from bytes or hex."""

        if isinstance(value, str):
            value = bytes.fromhex(value)
        return cls(value)

    @property
    def hash(self) -> bytes:
        """Hash of the message."""

        return blake2b(
            self.value,
            digest_size=Message.HASH_SIZE
        ).digest()

    def __bytes__(self) -> bytes:
        return self.value
