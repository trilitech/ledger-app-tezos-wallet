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
from typing import Any, Dict, List, Optional, Union

from pytezos.block.forge import forge_int_fixed
from pytezos.crypto.key import blake2b_32
from pytezos.michelson.forge import (
    forge_address,
    forge_array,
    forge_base58,
    forge_bool,
    forge_int32,
    forge_micheline,
    forge_nat,
    forge_public_key,
)
from pytezos.michelson.tags import prim_tags
from pytezos.operation.content import ContentMixin, format_mutez
import pytezos.operation.forge as forge_operation
from pytezos.operation.forge import reserved_entrypoints, forge_tag
from pytezos.rpc.kind import operation_tags

class Message(ABC):
    """Class representing a message."""

    @property
    def hash(self) -> bytes:
        """hash of the message."""
        return blake2b_32(bytes(self)).digest()

    @abstractmethod
    def __bytes__(self) -> bytes:
        raise NotImplementedError

    def __str__(self) -> str:
        return bytes(self).hex()

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
    BLOCK_HASH: str                      = 'BKiHLREqU3JkXfzEDYAkmmfX48gBDtYhMrpA98s7Aq4SzbUAB6M'
    PROTOCOL_HASH: str                   = 'PrihK96nBAFSxVL1GLJTVhu9YnzkMFiBeuJRPA8NwuZVZCE1L6i'
    ED25519_PUBLIC_KEY_HASH: str         = 'tz1Ke2h7sDdakHJQh8WX4Z372du1KChsksyU'
    ORIGINATED_ADDRESS: str              = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT'
    ORIGINATED_SMART_ROLLUP_ADDRESS: str = 'sr163Lv22CdE8QagCwf48PWDTquk6isQwv57'
    SMART_ROLLUP_COMMITMENT_HASH: str    = 'src12UJzB8mg7yU6nWPzicH7ofJbFjyJEbHvwtZdfRXi8DQHNp1LY8'
    ED25519_PUBLIC_KEY: str              = 'edpkteDwHwoNPB18tKToFKeSCykvr1ExnoMV5nawTJy9Y9nLTfQ541'
    SCRIPT_EXPR_HASH: str                = 'exprtWsu7N8st7XBhS685Qa2B4xP6TuTN9ve9UPCU29fV94ySDo5Va'
    ENTRYPOINT: str                      = 'default'
    BALLOT: str                          = 'yay'
    SMART_ROLLUP_KIND: str               = 'arith'

    class DefaultMicheline:
        """Class holding Micheline default values."""
        VALUE: Micheline = {'prim': 'Unit'}
        TYPE: Micheline  = {'prim': 'unit'}
        CODE: Micheline  = [{'prim': 'CDR'}, {'prim': 'NIL', 'args': [{'prim': 'operation'}]}, {'prim': 'PAIR'}]

class Watermark(IntEnum):
    """Class hodling messages watermark."""
    MANAGER_OPERATION    = 0x03
    MICHELINE_EXPRESSION = 0x05

# pytezos is not up to date with the protocol Seoul
# See `https://gitlab.com/tezos/tezos/-/blob/v23-release/src/proto_023_PtSeouLo/lib_protocol/michelson_v1_primitives.ml#L807`
prim_tags.update({ 'IS_IMPLICIT_ACCOUNT': b'\x9e'})

class MichelineExpr(Message):
    """Class representing a tezos micheline expression."""

    expr: Micheline

    def __init__(self, expr: Micheline):
        self.expr = expr

    def __bytes__(self) -> bytes:
        raw = b''
        raw += forge_int_fixed(Watermark.MICHELINE_EXPRESSION, 1)
        raw += forge_micheline(self.expr)
        return raw


class OperationBuilder(ContentMixin):
    """Class representing to extends and fix pytezos.ContentMixin."""

    def delegation(self, delegate, *args, **kwargs):
        delegation = super().delegation(delegate, *args, **kwargs)

        if delegate is None:
            delegation.pop('delegate')

        return delegation

    def set_deposit_limit(
            self,
            limit: Optional[int] = None,
            source: str = '',
            counter: int = 0,
            fee: int = 0,
            gas_limit: int = 0,
            storage_limit: int = 0):
        """Build a Tezos set-deposit-limit."""
        content = {
            'kind': 'set_deposit_limit',
            'source': source,
            'fee': format_mutez(fee),
            'counter': str(counter),
            'gas_limit': str(gas_limit),
            'storage_limit': str(storage_limit),
        }

        if limit is not None:
            content['limit'] = format_mutez(limit)

        return self.operation(content)

    def increase_paid_storage(
            self,
            amount: int = 0,
            destination: str = '',
            source: str = '',
            counter: int = 0,
            fee: int = 0,
            gas_limit: int = 0,
            storage_limit: int = 0):
        """Build a Tezos increase-paid-storage."""
        return self.operation(
            {
                'kind': 'increase_paid_storage',
                'source': source,
                'fee': format_mutez(fee),
                'counter': str(counter),
                'gas_limit': str(gas_limit),
                'storage_limit': str(storage_limit),
                'amount': str(amount),
                'destination': destination,
            }
        )

    def update_consensus_key(
            self,
            pk: str = '',
            proof: Optional[str] = None,
            source: str = '',
            counter: int = 0,
            fee: int = 0,
            gas_limit: int = 0,
            storage_limit: int = 0):
        """Build a Tezos update-consensus-key."""
        content = {
            'kind': 'update_consensus_key',
            'source': source,
            'fee': format_mutez(fee),
            'counter': str(counter),
            'gas_limit': str(gas_limit),
            'storage_limit': str(storage_limit),
            'pk': pk,
        }

        if proof is not None:
            content['proof'] = proof

        return self.operation(content)

    def update_companion_key(
            self,
            pk: str = '',
            proof: Optional[str] = None,
            source: str = '',
            counter: int = 0,
            fee: int = 0,
            gas_limit: int = 0,
            storage_limit: int = 0):
        """Build a Tezos update-companion-key."""
        content = {
            'kind': 'update_companion_key',
            'source': source,
            'fee': format_mutez(fee),
            'counter': str(counter),
            'gas_limit': str(gas_limit),
            'storage_limit': str(storage_limit),
            'pk': pk,
        }

        if proof is not None:
            content['proof'] = proof

        return self.operation(content)

    def smart_rollup_originate(
            self,
            pvm_kind: str = '',
            kernel: str = '',
            parameters_ty: Micheline = Default.DefaultMicheline.TYPE,
            whitelist: Optional[List[str]] = None,
            source: str = '',
            counter: int = 0,
            fee: int = 0,
            gas_limit: int = 0,
            storage_limit: int = 0):
        """Build a Tezos smart rollup originate."""
        content = {
            'kind': 'smart_rollup_originate',
            'source': source,
            'fee': format_mutez(fee),
            'counter': str(counter),
            'gas_limit': str(gas_limit),
            'storage_limit': str(storage_limit),
            'pvm_kind': pvm_kind,
            'kernel': kernel,
            'parameters_ty': parameters_ty,
        }

        if whitelist is not None:
            content['whitelist'] = whitelist

        return self.operation(content)

class OperationForge:
    """Class to helps forging Tezos operation."""

    # Insert new reserved entrypoint
    reserved_entrypoints['stake'] = b'\x06'
    reserved_entrypoints['unstake'] = b'\x07'
    reserved_entrypoints['finalize_unstake'] = b'\x08'
    reserved_entrypoints['delegate_parameters'] = b'\x09'

    # Insert new operation tag
    operation_tags['set_deposit_limit'] = 112
    operation_tags['increase_paid_storage'] = 113
    operation_tags['update_consensus_key'] = 114
    operation_tags['update_companion_key'] = 115
    operation_tags['smart_rollup_originate'] = 200

    failing_noop = forge_operation.forge_failing_noop
    transaction = forge_operation.forge_transaction
    origination = forge_operation.forge_origination
    delegation = forge_operation.forge_delegation
    register_global_constant = forge_operation.forge_register_global_constant
    transfer_ticket = forge_operation.forge_transfer_ticket
    smart_rollup_add_messages = forge_operation.forge_smart_rollup_add_messages
    smart_rollup_execute_outbox_message = forge_operation.forge_smart_rollup_execute_outbox_message

    # Fix Pytezos reveal forging updated in 3.16.0
    @staticmethod
    def reveal(content: Dict[str, Any]) -> bytes:
        """Forge a Tezos reveal."""
        res = forge_tag(operation_tags[content['kind']])
        res += forge_address(content['source'], tz_only=True)
        res += forge_nat(int(content['fee']))
        res += forge_nat(int(content['counter']))
        res += forge_nat(int(content['gas_limit']))
        res += forge_nat(int(content['storage_limit']))
        res += forge_public_key(content['public_key'])

        if content.get('proof'):
            res += forge_bool(True)
            res += forge_array(forge_base58(content['proof']))
        else:
            res += forge_bool(False)

        return res

    @staticmethod
    def proposals(content: Dict[str, Any]) -> bytes:
        """Forge a Tezos proposals."""
        res = forge_tag(operation_tags[content['kind']])
        res += forge_address(content['source'], tz_only=True)
        res += forge_int32(int(content['period']))
        res += forge_array(b''.join(map(forge_base58, content['proposals'])))
        return res

    BALLOT_TAG = { 'yay': 0, 'nay': 1, 'pass': 2 }

    @staticmethod
    def ballot(content: Dict[str, Any]) -> bytes:
        """Forge a Tezos ballot."""
        res = forge_tag(operation_tags[content['kind']])
        res += forge_address(content['source'], tz_only=True)
        res += forge_int32(int(content['period']))
        res += forge_base58(content['proposal'])
        res += forge_int_fixed(OperationForge.BALLOT_TAG[content['ballot']], 1)
        return res

    @staticmethod
    def set_deposit_limit(content: Dict[str, Any]) -> bytes:
        """Forge a Tezos set-deposit-limit."""
        res = forge_tag(operation_tags[content['kind']])
        res += forge_address(content['source'], tz_only=True)
        res += forge_nat(int(content['fee']))
        res += forge_nat(int(content['counter']))
        res += forge_nat(int(content['gas_limit']))
        res += forge_nat(int(content['storage_limit']))

        if content.get('limit'):
            res += forge_bool(True)
            res += forge_nat(int(content['limit']))
        else:
            res += forge_bool(False)

        return res

    @staticmethod
    def increase_paid_storage(content: Dict[str, Any]) -> bytes:
        """Forge a Tezos increase-paid-storage."""
        res = forge_tag(operation_tags[content['kind']])
        res += forge_address(content['source'], tz_only=True)
        res += forge_nat(int(content['fee']))
        res += forge_nat(int(content['counter']))
        res += forge_nat(int(content['gas_limit']))
        res += forge_nat(int(content['storage_limit']))
        res += forge_nat(int(content['amount']))
        res += forge_address(content['destination'])
        return res

    @staticmethod
    def update_consensus_key(content: Dict[str, Any]) -> bytes:
        """Forge a Tezos update-consensus-key."""
        res = forge_tag(operation_tags[content['kind']])
        res += forge_address(content['source'], tz_only=True)
        res += forge_nat(int(content['fee']))
        res += forge_nat(int(content['counter']))
        res += forge_nat(int(content['gas_limit']))
        res += forge_nat(int(content['storage_limit']))
        res += forge_public_key(content['pk'])

        if content.get('proof'):
            res += forge_bool(True)
            res += forge_array(forge_base58(content['proof']))
        else:
            res += forge_bool(False)

        return res

    @staticmethod
    def update_companion_key(content: Dict[str, Any]) -> bytes:
        """Forge a Tezos update-companion-key."""
        res = forge_tag(operation_tags[content['kind']])
        res += forge_address(content['source'], tz_only=True)
        res += forge_nat(int(content['fee']))
        res += forge_nat(int(content['counter']))
        res += forge_nat(int(content['gas_limit']))
        res += forge_nat(int(content['storage_limit']))
        res += forge_public_key(content['pk'])

        if content.get('proof'):
            res += forge_bool(True)
            res += forge_array(forge_base58(content['proof']))
        else:
            res += forge_bool(False)

        return res

    PVM_KIND_TAG = { 'arith': 0, 'wasm_2_0_0': 1, 'riscv': 2 }

    @staticmethod
    def smart_rollup_originate(content: Dict[str, Any]) -> bytes:
        """Forge a Tezos smart rollup originate."""
        res = forge_tag(operation_tags[content['kind']])
        res += forge_address(content['source'], tz_only=True)
        res += forge_nat(int(content['fee']))
        res += forge_nat(int(content['counter']))
        res += forge_nat(int(content['gas_limit']))
        res += forge_nat(int(content['storage_limit']))
        res += forge_int_fixed(
            OperationForge.PVM_KIND_TAG[content['pvm_kind']], 1
        )
        res += forge_array(bytes.fromhex(content['kernel']))
        res += forge_array(forge_micheline(content['parameters_ty']))

        if content.get('whitelist') is not None:
            res += forge_bool(True)
            res += forge_array(b''.join(
                forge_address(pkh, tz_only=True)
                for pkh in content['whitelist']
            ))
        else:
            res += forge_bool(False)

        return res

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

class Proposals(Operation):
    """Class representing a tezos proposals."""

    proposals_: List[str]
    source: str
    period: int

    def __init__(self,
                 proposals: List[str] = [Default.PROTOCOL_HASH],
                 source: str = Default.ED25519_PUBLIC_KEY_HASH,
                 period: int = 0,
                 **kwargs):
        self.proposals_ = proposals
        self.source = source
        self.period = period
        Operation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return OperationForge.proposals(
            self.proposals(
                self.proposals_,
                self.source,
                self.period
            )
        )

class Ballot(Operation):
    """Class representing a tezos ballot."""

    proposal : str
    ballot_ : str
    source : str
    period: int

    def __init__(self,
                 proposal: str = Default.PROTOCOL_HASH,
                 ballot: str = Default.BALLOT,
                 source: str = Default.ED25519_PUBLIC_KEY_HASH,
                 period: int = 0,
                 **kwargs):
        self.proposal = proposal
        self.ballot_ = ballot
        self.source = source
        self.period = period
        Operation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return OperationForge.ballot(
            self.ballot(
                self.proposal,
                self.ballot_,
                self.source,
                self.period
            )
        )

class FailingNoop(Operation):
    """Class representing a tezos failing-noop."""

    message: str

    def __init__(self, message: str = "", **kwargs):
        self.message = message
        Operation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return OperationForge.failing_noop(self.failing_noop(self.message))

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

class OperationGroup(Operation):
    """Class representing a group of tezos manager operation."""

    operations: List[ManagerOperation]

    def __init__(self,
                 operations: List[ManagerOperation] = [],
                 **kwargs):
        self.operations = operations
        Operation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return b''.join(map(lambda op: op.forge(), self.operations))

class Reveal(ManagerOperation):
    """Class representing a tezos reveal."""

    public_key: str
    proof: Optional[str]

    def __init__(self,
                 public_key: str = Default.ED25519_PUBLIC_KEY,
                 proof: Optional[str] = None,
                 **kwargs):
        self.public_key = public_key
        self.proof = proof
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return OperationForge.reveal(
            self.reveal(
                self.public_key,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit,
                self.proof
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
        return OperationForge.transaction(
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
        return OperationForge.origination(
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
        return OperationForge.delegation(
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
        return OperationForge.register_global_constant(
            self.register_global_constant(
                self.value,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )

class SetDepositLimit(ManagerOperation):
    """Class representing a tezos set deposit limit."""

    limit: Optional[int]

    def __init__(self,
                 limit: Optional[int] = None,
                 **kwargs):
        self.limit = limit
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return OperationForge.set_deposit_limit(
            self.set_deposit_limit(
                self.limit,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )

class IncreasePaidStorage(ManagerOperation):
    """Class representing a tezos increase paid storage."""

    amount: int
    destination: str

    def __init__(self,
                 amount: int = 0,
                 destination: str = Default.ORIGINATED_ADDRESS,
                 **kwargs):
        self.amount = amount
        self.destination = destination
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return OperationForge.increase_paid_storage(
            self.increase_paid_storage(
                self.amount,
                self.destination,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )

class UpdateConsensusKey(ManagerOperation):
    """Class representing a tezos update consensus key."""

    pk: str
    proof: Optional[str]

    def __init__(self,
                 pk: str = Default.ED25519_PUBLIC_KEY,
                 proof: Optional[str] = None,
                 **kwargs):
        self.pk = pk
        self.proof = proof
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return OperationForge.update_consensus_key(
            self.update_consensus_key(
                self.pk,
                self.proof,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )

class UpdateCompanionKey(ManagerOperation):
    """Class representing a tezos update companion key."""

    pk: str
    proof: Optional[str]

    def __init__(self,
                 pk: str = Default.ED25519_PUBLIC_KEY,
                 proof: Optional[str] = None,
                 **kwargs):
        self.pk = pk
        self.proof = proof
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return OperationForge.update_companion_key(
            self.update_companion_key(
                self.pk,
                self.proof,
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
        return OperationForge.transfer_ticket(
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

class ScRollupOriginate(ManagerOperation):
    """Class representing a tezos smart rollup originate."""

    pvm_kind: str
    kernel: str
    parameters_ty: Micheline
    whitelist: Optional[List[str]]

    def __init__(self,
                 pvm_kind: str = Default.SMART_ROLLUP_KIND,
                 kernel: str = "",
                 parameters_ty: Micheline = Default.DefaultMicheline.TYPE,
                 whitelist: Optional[List[str]] = None,
                 **kwargs):
        self.pvm_kind = pvm_kind
        self.kernel = kernel
        self.parameters_ty = parameters_ty
        self.whitelist = whitelist
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return OperationForge.smart_rollup_originate(
            self.smart_rollup_originate(
                self.pvm_kind,
                self.kernel,
                self.parameters_ty,
                self.whitelist,
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
        return OperationForge.smart_rollup_add_messages(
            self.smart_rollup_add_messages(
                self.message,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )

class ScRollupExecuteOutboxMessage(ManagerOperation):
    """Class representing a tezos smart rollup execute outbox message."""

    rollup: str
    cemented_commitment: str
    output_proof: bytes

    def __init__(self,
                 rollup: str = Default.ORIGINATED_SMART_ROLLUP_ADDRESS,
                 cemented_commitment: str = Default.SMART_ROLLUP_COMMITMENT_HASH,
                 output_proof: bytes = b'',
                 **kwargs):
        self.rollup = rollup
        self.cemented_commitment = cemented_commitment
        self.output_proof = output_proof
        ManagerOperation.__init__(self, **kwargs)

    def forge(self) -> bytes:
        return OperationForge.smart_rollup_execute_outbox_message(
            self.smart_rollup_execute_outbox_message(
                self.rollup,
                self.cemented_commitment,
                self.output_proof,
                self.source,
                self.counter,
                self.fee,
                self.gas_limit,
                self.storage_limit
            )
        )
