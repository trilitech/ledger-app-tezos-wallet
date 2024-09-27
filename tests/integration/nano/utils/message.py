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
from enum import IntEnum
from typing import Dict, List, Union

from pytezos.block.forge import forge_int_fixed
from pytezos.crypto.key import blake2b_32
from pytezos.michelson.forge import forge_base58
from pytezos.operation.content import ContentMixin
from pytezos.operation.forge import (
    reserved_entrypoints,
    forge_failing_noop,
    forge_transaction,
)

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


Micheline = Union[List, Dict]

class Default:
    """Class holding default values."""
    BLOCK_HASH: str              = 'BKiHLREqU3JkXfzEDYAkmmfX48gBDtYhMrpA98s7Aq4SzbUAB6M'
    ED25519_PUBLIC_KEY_HASH: str = 'tz1Ke2h7sDdakHJQh8WX4Z372du1KChsksyU'
    ENTRYPOINT: str              = 'default'

    class DefaultMicheline:
        """Class holding Micheline default values."""
        VALUE: Micheline = {'prim': 'Unit'}

class Watermark(IntEnum):
    """Class hodling messages watermark."""
    MANAGER_OPERATION = 0x03


# Insert new reserved entrypoint
reserved_entrypoints['stake'] = b'\x06'
reserved_entrypoints['unstake'] = b'\x07'
reserved_entrypoints['finalize_unstake'] = b'\x08'
reserved_entrypoints['delegate_parameters'] = b'\x09'

class Operation(Message, ContentMixin):
    """Class representing a tezos operation."""

    branch: str

    def __init__(self, branch: str = Default.BLOCK_HASH):
        self.branch = branch

    @abstractmethod
    def forge(self) -> bytes:
        """Forge the operation."""
        raise NotImplementedError

    def __bytes__(self) -> bytes:
        raw = b''
        raw += forge_int_fixed(Watermark.MANAGER_OPERATION, 1)
        raw += forge_base58(self.branch)
        raw += self.forge()
        return raw

class FailingNoop(Operation):
    """Class representing a tezos failing-noop."""

    message: str

    def __init__(self, message: str = "", **kwargs):
        self.message = message
        Operation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return forge_failing_noop(self.failing_noop(self.message))

class ManagerOperation(Operation):
    """Class representing a tezos manager operation."""

    source: str
    fee: int
    counter: int
    gas_limit: int
    storage_limit: int

    def __init__(self,
                 source: str = Default.ED25519_PUBLIC_KEY_HASH,
                 fee: int = 0,
                 counter: int = 0,
                 gas_limit: int = 0,
                 storage_limit: int = 0,
                 **kwargs):
        self.source = source
        self.fee = fee
        self.counter = counter
        self.gas_limit = gas_limit
        self.storage_limit = storage_limit
        Operation.__init__(self, **kwargs)

class Transaction(ManagerOperation):
    """Class representing a tezos transaction."""

    destination: str
    amount: int
    entrypoint: str
    parameter: Micheline

    def __init__(self,
                 destination: str = Default.ED25519_PUBLIC_KEY_HASH,
                 amount: int = 0,
                 entrypoint: str = Default.ENTRYPOINT,
                 parameter: Micheline = Default.DefaultMicheline.VALUE,
                 **kwargs):
        self.destination = destination
        self.amount = amount
        self.entrypoint = entrypoint
        self.parameter = parameter
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        parameters = { "entrypoint": self.entrypoint, "value": self.parameter }
        return forge_transaction(
            self.transaction(
                self.destination,
                self.amount,
                parameters,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )
