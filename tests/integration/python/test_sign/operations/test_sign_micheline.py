#!/usr/bin/env python3
# Copyright 2024 Functori <contact@functori.com>
# Copyright 2024 Trilitech <contact@trili.tech>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Gathering of tests related to Micheline."""

from pathlib import Path
from string import ascii_lowercase, ascii_uppercase, digits
from typing import List, Tuple

import pytest
from _pytest.mark import ParameterSet

from utils.account import Account
from utils.backend import TezosBackend
from utils.message import Default, MichelineExpr, Micheline
from utils.navigator import TezosNavigator

sequence_cases: List[Tuple[Micheline, str]] = [
    ([], "empty"),
    ([{"prim": "Unit"}], "singleton"),
    ([{"int": i} for i in range(0, 256)], "long"),
]

int_cases: List[Tuple[Micheline, str]] = [
    ({"int": 0}, "0"),
    ({"int": 123456789}, "all-digit"),
    ({"int": 1000000000000000000}, "very-big"),
    ({"int": -1}, "neg"),
    ({"int": -1000000000000000000}, "very-small"),
]

bytes_cases: List[Tuple[Micheline, str]] = [
    ({"bytes": b''.hex()}, "empty"),
    ({"bytes": b'00'.hex()}, "0"),
    ({"bytes": bytes(range(0, 256)).hex()}, "all-byte"),
]

string_cases: List[Tuple[Micheline, str]] = [
    ({"string": ""}, "empty"),
    ({"string": ''.join(map(chr, range(0, 256)))}, "first-256-char"),
]

default_view_interface = ({"prim": "nat"}, {"prim": "unit"}, [{"prim": "DROP"}, {"prim": "UNIT"}])

prim_cases: List[Tuple[Micheline, str]] = [

    # Keyword
    ({"prim": "parameter", 'args': [{"prim": "unit"}]}, "parameter"),
    ({"prim": "storage", 'args': [{"prim": "unit"}]}, "storage"),
    ({"prim": "code", 'args': [{"prim": "ADD"}]}, "code"),
    ({"prim": "view", 'args': [{'string': ''}, *default_view_interface]}, "small-view-name"),
    ({"prim": "view", 'args': [{'string': 'v' * 31}, *default_view_interface]}, "long-view-name"),
    ({"prim": "view", 'args': [{'string': 'view\nname'}, *default_view_interface]}, "view-name-with-newline"),

    # Data
    ({"prim": "False"}, "False"),
    ({"prim": "Elt", 'args': [{"prim": "Unit"}, {"prim": "Unit"}]}, "Elt"),
    ({"prim": "Left", 'args': [{"prim": "Unit"}]}, "Left"),
    ({"prim": "None"}, "None"),
    ({"prim": "Pair", 'args': [{"prim": "Unit"}, {"prim": "Unit"}]}, "Pair"),
    ({"prim": "Pair", 'args': [{"prim": "Unit"}, {"prim": "Unit"}, {"prim": "Unit"}, {"prim": "Unit"}]}, "long-Pair"),
    ({"prim": "Right", 'args': [{"prim": "Unit"}]}, "Right"),
    ({"prim": "Some", 'args': [{"prim": "Unit"}]}, "Some"),
    ({"prim": "True"}, "True"),
    ({"prim": "Unit"}, "Unit"),
    ({"prim": "Ticket", 'args': [{"string": Default.ORIGINATED_ADDRESS+"%my_entrypoint"}, {"prim": "unit"}, {"prim": "Unit"}, {"int": 0}]}, "Ticket"),
    ({"prim": "Lambda_rec", 'args': [[{"prim": "DROP"}, {"prim": "DROP"}, {"prim": "UNIT"}]]}, "Lambda_rec"),

    # Instruction
    ({"prim": "PACK"}, "PACK"),
    ({"prim": "UNPACK", 'args': [{"prim": "unit"}]}, "UNPACK"),
    ({"prim": "BLAKE2B"}, "BLAKE2B"),
    ({"prim": "SHA256"}, "SHA256"),
    ({"prim": "SHA512"}, "SHA512"),
    ({"prim": "ABS"}, "ABS"),
    ({"prim": "ADD"}, "ADD"),
    ({"prim": "AMOUNT"}, "AMOUNT"),
    ({"prim": "AND"}, "AND"),
    ({"prim": "BALANCE"}, "BALANCE"),
    ({"prim": "CAR"}, "CAR"),
    ({"prim": "CDR"}, "CDR"),
    ({"prim": "CHAIN_ID"}, "CHAIN_ID"),
    ({"prim": "CHECK_SIGNATURE"}, "CHECK_SIGNATURE"),
    ({"prim": "COMPARE"}, "COMPARE"),
    ({"prim": "CONCAT"}, "CONCAT"),
    ({"prim": "CONS"}, "CONS"),
    ({"prim": "__CREATE_ACCOUNT__"}, "CREATE_ACCOUNT"),  # deprecated
    ({"prim": "CREATE_CONTRACT", 'args': [[
        {"prim": "parameter", 'args': [{"prim": "unit"}]},
        {"prim": "storage", 'args': [{"prim": "unit"}]},
        {"prim": "code", 'args': [{"prim": "ADD"}]},
    ]]}, "CREATE_CONTRACT"),
    ({"prim": "CREATE_CONTRACT", 'args': [[
        {"prim": "parameter", 'args': [{"prim": "unit"}]},
        {"prim": "storage", 'args': [{"prim": "unit"}]},
        {"prim": "code", 'args': [{"prim": "ADD"}]},
        {"prim": "view", 'args': [{'string': 'view-name1'}, *default_view_interface]},
        {"prim": "view", 'args': [{'string': 'view-name2'}, *default_view_interface]},
    ]]}, "CREATE_CONTRACT-with-views"),
    ({"prim": "IMPLICIT_ACCOUNT"}, "IMPLICIT_ACCOUNT"),
    ({"prim": "DIP", 'args': [[{"prim": "UNIT"}]]}, "DIP"),
    ({"prim": "DIP", 'args': [{"int": 0x3FF}, [{"prim": "UNIT"}]]}, "DIP-n"),
    ({"prim": "DROP", 'args': [{"int": 0x3FF}]}, "DROP"),
    ({"prim": "DROP"}, "DROP-no-arg"),
    ({"prim": "DUP", 'args': [{"int": 0x3FF}]}, "DUP"),
    ({"prim": "DUP"}, "DUP-no-arg"),
    ({"prim": "VIEW", 'args': [{'string': 'view-name'}, {"prim": "unit"}]}, "VIEW"),
    ({"prim": "EDIV"}, "EDIV"),
    ({"prim": "EMPTY_BIG_MAP", 'args': [{"prim": "unit"}, {"prim": "unit"}]}, "EMPTY_BIG_MAP"),
    ({"prim": "EMPTY_MAP", 'args': [{"prim": "unit"}, {"prim": "unit"}]}, "EMPTY_MAP"),
    ({"prim": "EMPTY_SET", 'args': [{"prim": "unit"}]}, "EMPTY_SET"),
    ({"prim": "EQ"}, "EQ"),
    ({"prim": "EXEC"}, "EXEC"),
    ({"prim": "APPLY"}, "APPLY"),
    ({"prim": "FAILWITH"}, "FAILWITH"),
    ({"prim": "GE"}, "GE"),
    ({"prim": "GET", 'args': [{"int": 0x7FF}]}, "GET"),
    ({"prim": "GET"}, "GET-no-arg"),
    ({"prim": "GET_AND_UPDATE"}, "GET_AND_UPDATE"),
    ({"prim": "GT"}, "GT"),
    ({"prim": "HASH_KEY"}, "HASH_KEY"),
    ({"prim": "IF", 'args': [[{"prim": "UNIT"}], [{"prim": "UNIT"}]]}, "IF"),
    ({"prim": "IF_CONS", 'args': [[{"prim": "UNIT"}], [{"prim": "UNIT"}]]}, "IF_CONS"),
    ({"prim": "IF_LEFT", 'args': [[{"prim": "UNIT"}], [{"prim": "UNIT"}]]}, "IF_LEFT"),
    ({"prim": "IF_NONE", 'args': [[{"prim": "UNIT"}], [{"prim": "UNIT"}]]}, "IF_NONE"),
    ({"prim": "INT"}, "INT"),
    ({"prim": "LAMBDA", 'args': [{"prim": "unit"}, {"prim": "unit"}, [{"prim": "UNIT"}]]}, "LAMBDA"),
    ({"prim": "LAMBDA_REC", 'args': [{"prim": "unit"}, {"prim": "unit"}, [{"prim": "UNIT"}]]}, "LAMBDA_REC"),
    ({"prim": "LE"}, "LE"),
    ({"prim": "LEFT", 'args': [{"prim": "unit"}]}, "LEFT"),
    ({"prim": "LEVEL"}, "LEVEL"),
    ({"prim": "LOOP", 'args': [[{"prim": "UNIT"}]]}, "LOOP"),
    ({"prim": "LSL"}, "LSL"),
    ({"prim": "LSR"}, "LSR"),
    ({"prim": "LT"}, "LT"),
    ({"prim": "MAP", 'args': [[{"prim": "UNIT"}]]}, "MAP"),
    ({"prim": "MEM"}, "MEM"),
    ({"prim": "MUL"}, "MUL"),
    ({"prim": "NEG"}, "NEG"),
    ({"prim": "NEQ"}, "NEQ"),
    ({"prim": "NIL", 'args': [{"prim": "unit"}]}, "NIL"),
    ({"prim": "NONE", 'args': [{"prim": "unit"}]}, "NONE"),
    ({"prim": "NOT"}, "NOT"),
    ({"prim": "NOW"}, "NOW"),
    ({"prim": "MIN_BLOCK_TIME"}, "MIN_BLOCK_TIME"),
    ({"prim": "OR"}, "OR"),
    ({"prim": "PAIR", 'args': [{"int": 0x3FF}]}, "PAIR"),
    ({"prim": "PAIR"}, "PAIR-no-arg"),
    ({"prim": "UNPAIR", 'args': [{"int": 0}]}, "UNPAIR-min"),
    ({"prim": "UNPAIR", 'args': [{"int": 0x3FF}]}, "UNPAIR-max"),
    ({"prim": "UNPAIR"}, "UNPAIR-no-arg"),
    ({"prim": "PUSH", 'args': [{"prim": "unit"}, {"prim": "Unit"}]}, "PUSH"),
    ({"prim": "RIGHT", 'args': [{"prim": "unit"}]}, "RIGHT"),
    ({"prim": "SIZE"}, "SIZE"),
    ({"prim": "SOME"}, "SOME"),
    ({"prim": "SOURCE"}, "SOURCE"),
    ({"prim": "SENDER"}, "SENDER"),
    ({"prim": "SELF"}, "SELF"),
    ({"prim": "SELF_ADDRESS"}, "SELF_ADDRESS"),
    ({"prim": "SLICE"}, "SLICE"),
    ({"prim": "STEPS_TO_QUOTA"}, "STEPS_TO_QUOTA"),  # deprecated
    ({"prim": "SUB"}, "SUB"),
    ({"prim": "SUB_MUTEZ"}, "SUB_MUTEZ"),
    ({"prim": "SWAP"}, "SWAP"),
    ({"prim": "TRANSFER_TOKENS"}, "TRANSFER_TOKENS"),
    ({"prim": "SET_DELEGATE"}, "SET_DELEGATE"),
    ({"prim": "UNIT"}, "UNIT"),
    ({"prim": "UPDATE", 'args': [{"int": 0}]}, "UPDATE-min"),
    ({"prim": "UPDATE", 'args': [{"int": 0x7FF}]}, "UPDATE-max"),
    ({"prim": "UPDATE"}, "UPDATE-no-arg"),
    ({"prim": "XOR"}, "XOR"),
    ({"prim": "ITER", 'args': [[{"prim": "UNIT"}]]}, "ITER"),
    ({"prim": "LOOP_LEFT", 'args': [[{"prim": "UNIT"}]]}, "LOOP_LEFT"),
    ({"prim": "ADDRESS"}, "ADDRESS"),
    ({"prim": "CONTRACT", 'args': [{"prim": "unit"}]}, "CONTRACT"),
    ({"prim": "ISNAT"}, "ISNAT"),
    ({"prim": "CAST", 'args': [{"prim": "unit"}]}, "CAST"),
    ({"prim": "RENAME"}, "RENAME"),
    ({"prim": "SAPLING_EMPTY_STATE", 'args': [{"int": 0XFFFF}]}, "SAPLING_EMPTY_STATE"),
    ({"prim": "SAPLING_VERIFY_UPDATE"}, "SAPLING_VERIFY_UPDATE"),
    ({"prim": "DIG", 'args': [{"int": 0x3FF}]}, "DIG"),
    ({"prim": "DUG", 'args': [{"int": 0x3FF}]}, "DUG"),
    ({"prim": "NEVER"}, "NEVER"),
    ({"prim": "VOTING_POWER"}, "VOTING_POWER"),
    ({"prim": "TOTAL_VOTING_POWER"}, "TOTAL_VOTING_POWER"),
    ({"prim": "KECCAK"}, "KECCAK"),
    ({"prim": "SHA3"}, "SHA3"),
    ({"prim": "PAIRING_CHECK"}, "PAIRING_CHECK"),
    ({"prim": "TICKET"}, "TICKET"),
    ({"prim": "TICKET_DEPRECATED"}, "TICKET_DEPRECATED"),
    ({"prim": "READ_TICKET"}, "READ_TICKET"),
    ({"prim": "SPLIT_TICKET"}, "SPLIT_TICKET"),
    ({"prim": "JOIN_TICKETS"}, "JOIN_TICKETS"),
    ({"prim": "OPEN_CHEST"}, "OPEN_CHEST"),
    ({"prim": "EMIT", 'args': [{"prim": "unit"}]}, "EMIT"),
    ({"prim": "EMIT"}, "EMIT-not-type"),
    ({"prim": "BYTES"}, "BYTES"),
    ({"prim": "NAT"}, "NAT"),
    ({"prim": "IS_IMPLICIT_ACCOUNT"}, "IS_IMPLICIT_ACCOUNT"),
    ({"prim": "INDEX_ADDRESS"}, "INDEX_ADDRESS"),
    ({"prim": "GET_ADDRESS_INDEX"}, "GET_ADDRESS_INDEX"),

    # Type
    ({"prim": "bool"}, "bool"),
    ({"prim": "contract", 'args': [{"prim": "unit"}]}, "contract"),
    ({"prim": "int"}, "int"),
    ({"prim": "key"}, "key"),
    ({"prim": "key_hash"}, "key_hash"),
    ({"prim": "lambda", 'args': [{"prim": "unit"}, {"prim": "unit"}]}, "lambda"),
    ({"prim": "list", 'args': [{"prim": "unit"}]}, "list"),
    ({"prim": "map", 'args': [{"prim": "unit"}, {"prim": "unit"}]}, "map"),
    ({"prim": "big_map", 'args': [{"prim": "unit"}, {"prim": "unit"}]}, "big_map"),
    ({"prim": "nat"}, "nat"),
    ({"prim": "option", 'args': [{"prim": "unit"}]}, "option"),
    ({"prim": "or", 'args': [{"prim": "unit"}, {"prim": "unit"}]}, "or"),
    ({"prim": "pair", 'args': [{"prim": "unit"}, {"prim": "unit"}]}, "pair"),
    ({"prim": "pair", 'args': [{"prim": "unit"}, {"prim": "unit"}, {"prim": "unit"}, {"prim": "unit"}]}, "long-pair"),
    ({"prim": "set", 'args': [{"prim": "unit"}]}, "set"),
    ({"prim": "signature"}, "signature"),
    ({"prim": "string"}, "string"),
    ({"prim": "bytes"}, "bytes"),
    ({"prim": "mutez"}, "mutez"),
    ({"prim": "timestamp"}, "timestamp"),
    ({"prim": "unit"}, "unit"),
    ({"prim": "operation"}, "operation"),
    ({"prim": "address"}, "address"),
    ({"prim": "tx_rollup_l2_address"}, "tx_rollup_l2_address"),  # deprecated
    ({"prim": "sapling_transaction", 'args': [{"int": 0}]}, "sapling_transaction"),
    ({"prim": "sapling_transaction", 'args': [{"int": 0XFFFF}]}, "sapling_transaction-max-memo-size"),
    ({"prim": "sapling_transaction_deprecated", 'args': [{"int": 0XFFFF}]}, "sapling_transaction_deprecated"),
    ({"prim": "sapling_state", 'args': [{"int": 0}]}, "sapling_state"),
    ({"prim": "chain_id"}, "chain_id"),
    ({"prim": "never"}, "never"),
    ({"prim": "bls12_381_g1"}, "bls12_381_g1"),
    ({"prim": "bls12_381_g2"}, "bls12_381_g2"),
    ({"prim": "bls12_381_fr"}, "bls12_381_fr"),
    ({"prim": "ticket", 'args': [{"prim": "unit"}]}, "ticket"),
    ({"prim": "chest_key"}, "chest_key"),
    ({"prim": "chest"}, "chest"),

    # Constant
    ({"prim": "constant", 'args': [{"string": Default.SCRIPT_EXPR_HASH}]}, "constant"),

    # Annots
    ({"prim": "unit", "annots": [":annot"]}, "with-annot"),
    ({"prim": "PAIR", "annots": ["@annot1", "%@annot2"]}, "with-annots"),
    ({"prim": "LEFT", "annots": ["%annot"], 'args': [{"prim": "unit"}]}, "with-annot-and-arg"),
    ({"prim": "unit", "annots": [":" + ascii_lowercase + ascii_uppercase + '_' + digits]}, "allowed-char-annot"),
    ({"prim": "unit", "annots": [":" + (ascii_lowercase * ((254 // len(ascii_lowercase)) + 1))[:254]]}, "max-len-annot"),

]

real_cases: List[Tuple[Micheline, str]] = [

    ({"prim": "pair", 'args': [
        {"prim": "pair", "annots": [":payload"], 'args': [
            {"prim": "nat", "annots": [":counter"]},
            {"prim": "or", "annots": [":action"], 'args': [
                {"prim": "pair", "annots": [":transfer"], 'args': [
                    {"prim": "mutez", "annots": [":amount"]},
                    {"prim": "contract", "annots": [":dest"], 'args': [{"prim": "unit"}]}
                ]},
                {"prim": "or", 'args': [
                    {"prim": "option", "annots": [":delegate"], 'args': [{"prim": "key_hash"}]},
                    {"prim": "pair", "annots": [":change_keys"], 'args': [
                        {"prim": "nat", "annots": [":threshold"]},
                        {"prim": "list", "annots": [":keys"], 'args': [{"prim": "key"}]}
                    ]}
                ]}
            ]}
        ]},
        {"prim": "list", "annots": [":sigs"], 'args': [{"prim": "option", 'args': [{"prim": "signature"}]}]}
    ]}, "type"),

    ({"prim": "Pair", 'args': [
        {"prim": "Pair", 'args': [
            {"int": 42},
            {"prim": "Left", 'args': [
                {"prim": "Pair", 'args': [
                    {"int": 123456789},
                    {"string": "tz1Ke2h7sDdakHJQh8WX4Z372du1KChsksyU"}
                ]},
            ]},
        ]},
        [
            {"prim": "Some", 'args': [
                {"string": "edsigtXomBKi5CTRf5cjATJWSyaRvhfYNHqSUGrn4SdbYRcGwQrUGjzEfQDTuqHhuA8b2d8NarZjz8TRf65WkpQmo423BtomS8Q"}
            ]},
            {"prim": "None"}
        ]
    ]}, "data"),

    ([
        {"prim": "UNPAIR"}, {"prim": "SWAP"}, {"prim": "DUP"}, {"prim": "DIP", 'args': [[{"prim": "SWAP"}]]},
        {"prim": "DIP", 'args': [[
            {"prim": "UNPAIR"},
            {"prim": "DUP"}, {"prim": "SELF"}, {"prim": "ADDRESS"}, {"prim": "CHAIN_ID"}, {"prim": "PAIR"}, {"prim": "PAIR"},
            {"prim": "PACK"},
            {"prim": "DIP", 'args': [[{"prim": "UNPAIR", "annots": ["@counter"]}, {"prim": "DIP", 'args': [[{"prim": "SWAP"}]]}]]}, {"prim": "SWAP"},
        ]]},
        {"prim": "UNPAIR", "annots": ["@stored_counter"]}, {"prim": "DIP", 'args': [[{"prim": "SWAP"}]]},
        {"prim": "COMPARE"}, {"prim": "EQ"}, {"prim": "IF", 'args': [[], [{"prim": "UNIT"}, {"prim": "FAILWITH"}]]},
        {"prim": "DIP", 'args': [[{"prim": "SWAP"}]]}, {"prim": "UNPAIR", "annots": ["@threshold", "@keys"]},
        {"prim": "DIP", 'args': [[
            {"prim": "PUSH", "annots": ["@valid"], 'args': [{"prim": "nat"}, {"int": 0}]}, {"prim": "SWAP"},
            {"prim": "ITER", 'args': [[
                {"prim": "DIP", 'args': [[{"prim": "SWAP"}]]}, {"prim": "SWAP"},
                {"prim": "IF_CONS", 'args': [
                    [{"prim": "IF_NONE", 'args': [
                        [{"prim": "SWAP"}, {"prim": "DROP"}],
                        [
                            {"prim": "SWAP"},
                            {"prim": "DIP", 'args': [[
                                {"prim": "SWAP"}, {"prim": "DIP", 'args': [[{"prim": "DIP", 'args': [[{"prim": "DUP"}]]}, {"prim": "SWAP"}]]},
                                {"prim": "CHECK_SIGNATURE"}, {"prim": "IF", 'args': [[], [{"prim": "UNIT"}, {"prim": "FAILWITH"}]]},
                                {"prim": "PUSH", 'args': [{"prim": "nat"}, {"int": 1}]}, {"prim": "ADD", "annots": ["@valid"]},
                            ]]},
                        ],
                    ]}],
                    [{"prim": "UNIT"}, {"prim": "FAILWITH"}],
                ]},
                {"prim": "SWAP"},
            ]]},
        ]]},
        {"prim": "COMPARE"}, {"prim": "LE"}, {"prim": "IF", 'args': [[], [{"prim": "UNIT"}, {"prim": "FAILWITH"}]]},
        {"prim": "DROP"}, {"prim": "DROP"},
        {"prim": "DIP", 'args': [[{"prim": "UNPAIR"}, {"prim": "PUSH", 'args': [{"prim": "nat"}, {"int": 1}]}, {"prim": "ADD", "annots": ["@new_counter"]}, {"prim": "PAIR"}]]},
        {"prim": "NIL", 'args': [{"prim": "operation"}]}, {"prim": "SWAP"},
        {"prim": "IF_LEFT", 'args': [
            [{"prim": "UNPAIR"}, {"prim": "UNIT"}, {"prim": "TRANSFER_TOKENS"}, {"prim": "CONS"}],
            [{"prim": "IF_LEFT", 'args': [
                [{"prim": "SET_DELEGATE"}, {"prim": "CONS"}],
                [{"prim": "DIP", 'args': [[{"prim": "SWAP"}, {"prim": "CAR"}]]}, {"prim": "SWAP"}, {"prim": "PAIR"}, {"prim": "SWAP"}],
            ]}],
        ]},
        {"prim": "PAIR"},
    ], "code"),

]


def _param(micheline: Micheline, name: str, category: str) -> ParameterSet:
    return pytest.param(micheline, id=f'{category}-{name}')


def _params(cases: List[Tuple[Micheline, str]], category: str) -> List[ParameterSet]:
    return [_param(*case_, category) for case_ in cases]


def _all_params(cases: List[Tuple[List[Tuple[Micheline, str]], str]]) -> List[ParameterSet]:
    return sum([_params(*cases_) for cases_ in cases], [])


@pytest.mark.parametrize(
    "micheline", _all_params([
        (sequence_cases, "sequence"),
        (int_cases, "int"),
        (bytes_cases, "bytes"),
        (string_cases, "string"),
        (prim_cases, "prim"),
        (real_cases, "real"),
    ])
)
def test_sign_micheline(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path,
        micheline: Micheline
):
    """Check signing micheline"""

    message = MichelineExpr(micheline)

    with backend.sign(account, message, with_hash=False) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=False,
        data=result.value
    )
