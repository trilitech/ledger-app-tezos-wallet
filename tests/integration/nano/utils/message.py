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

from abc import ABC, abstractmethod
from typing import Union

from pytezos.crypto.key import blake2b_32

class Message(ABC):
    """Class representing a message."""

    @property
    def hash(self) -> bytes:
        """hash of the message."""
        return blake2b_32(bytes(self)).digest()

    @abstractmethod
    def __bytes__(self) -> bytes:
        raise NotImplementedError

class RawMessage(Message):
    """Class representing a raw message."""

    _value: bytes

    def __init__(self, value: Union[str, bytes]):
        self._value = value if isinstance(value, bytes) else \
            bytes.fromhex(value)

    def __bytes__(self) -> bytes:
        return self._value
