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
from typing import Dict, List, Optional, Union

from pytezos.block.forge import forge_int_fixed
from pytezos.crypto.key import blake2b_32
from pytezos.michelson.forge import forge_base58
from pytezos.operation.content import ContentMixin
from pytezos.operation.forge import (
    reserved_entrypoints,
    forge_failing_noop,
    forge_transaction,
    forge_reveal,
    forge_origination,
    forge_delegation,
    forge_register_global_constant,
    forge_transfer_ticket,
    forge_smart_rollup_add_messages,
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
    ORIGINATED_ADDRESS: str      = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT'
    ED25519_PUBLIC_KEY: str      = 'edpkteDwHwoNPB18tKToFKeSCykvr1ExnoMV5nawTJy9Y9nLTfQ541'
    ENTRYPOINT: str              = 'default'

    class DefaultMicheline:
        """Class holding Micheline default values."""
        VALUE: Micheline = {'prim': 'Unit'}
        TYPE: Micheline  = {'prim': 'unit'}
        CODE: Micheline  = [{'prim': 'CDR'}, {'prim': 'NIL', 'args': [{'prim': 'operation'}]}, {'prim': 'PAIR'}]

class Watermark(IntEnum):
    """Class hodling messages watermark."""
    MANAGER_OPERATION = 0x03


# Insert new reserved entrypoint
reserved_entrypoints['stake'] = b'\x06'
reserved_entrypoints['unstake'] = b'\x07'
reserved_entrypoints['finalize_unstake'] = b'\x08'
reserved_entrypoints['delegate_parameters'] = b'\x09'

class OperationBuilder(ContentMixin):
    """Class representing to extends and fix pytezos.ContentMixin."""

    def delegation(self, delegate, *args, **kwargs):
        delegation = super().delegation(delegate, *args, **kwargs)

        if delegate is None:
            delegation.pop('delegate')

        return delegation


class Operation(Message, OperationBuilder):
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

class Reveal(ManagerOperation):
    """Class representing a tezos reveal."""

    public_key: str

    def __init__(self,
                 public_key: str = Default.ED25519_PUBLIC_KEY,
                 **kwargs):
        self.public_key = public_key
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return forge_reveal(
            self.reveal(
                self.public_key,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )

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

class Origination(ManagerOperation):
    """Class representing a tezos origination."""

    code: Micheline
    storage: Micheline
    balance: int
    delegate: Optional[str]

    def __init__(self,
                 code: Micheline = Default.DefaultMicheline.CODE,
                 storage: Micheline = Default.DefaultMicheline.TYPE,
                 balance: int = 0,
                 delegate: Optional[str] = None,
                 **kwargs):
        self.code = code
        self.storage = storage
        self.balance = balance
        self.delegate = delegate
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        script = { "code": self.code, "storage": self.storage }
        return forge_origination(
            self.origination(
                script,
                self.balance,
                self.delegate,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )

class Delegation(ManagerOperation):
    """Class representing a tezos delegation."""

    delegate: Optional[str]

    def __init__(self,
                 delegate: Optional[str] = None,
                 **kwargs):
        self.delegate = delegate
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return forge_delegation(
            self.delegation(
                self.delegate,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )

class RegisterGlobalConstant(ManagerOperation):
    """Class representing a tezos register global constant."""

    value: Micheline

    def __init__(self,
                 value: Micheline = Default.DefaultMicheline.VALUE,
                 **kwargs):
        self.value = value
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return forge_register_global_constant(
            self.register_global_constant(
                self.value,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )

class TransferTicket(ManagerOperation):
    """Class representing a tezos transfer ticket."""

    ticket_contents: Micheline
    ticket_ty: Micheline
    ticket_ticketer: str
    ticket_amount: int
    destination: str
    entrypoint: str

    def __init__(self,
                 ticket_contents: Micheline = Default.DefaultMicheline.VALUE,
                 ticket_ty: Micheline = Default.DefaultMicheline.TYPE,
                 ticket_ticketer: str = Default.ORIGINATED_ADDRESS,
                 ticket_amount: int = 0,
                 destination: str = Default.ORIGINATED_ADDRESS,
                 entrypoint: str = Default.ENTRYPOINT,
                 **kwargs):
        self.ticket_contents = ticket_contents
        self.ticket_ty = ticket_ty
        self.ticket_ticketer = ticket_ticketer
        self.ticket_amount = ticket_amount
        self.destination = destination
        self.entrypoint = entrypoint
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return forge_transfer_ticket(
            self.transfer_ticket(
                self.ticket_contents,
                self.ticket_ty,
                self.ticket_ticketer,
                self.ticket_amount,
                self.destination,
                self.entrypoint,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )

class ScRollupAddMessage(ManagerOperation):
    """Class representing a tezos smart rollup add message."""

    message: List[bytes]

    def __init__(self,
                 message: List[bytes] = [b''],
                 **kwargs):
        self.message = message
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return forge_smart_rollup_add_messages(
            self.smart_rollup_add_messages(
                self.message,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )
