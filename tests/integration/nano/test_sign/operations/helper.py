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

"""Helpers related to operations testing."""

from abc import ABC, abstractmethod
from pathlib import Path
from typing import Any, Dict, List, Optional, Type, TypeVar

import pytest

from ragger.navigator import NavInsID

from utils.account import Account
from utils.backend import TezosBackend
from utils.message import Operation
from utils.navigator import TezosNavigator


Op = TypeVar('Op', bound=Operation)


class Flow:
    """Data required to run `test_operation_flow`.

    name: str
    Identifier for naming the test: flow-<name>

    fields: Dict[str, Any]
    Fields passed to the operation contstructor

    """

    name: str
    fields: Dict[str, Any]

    def __init__(self, name, **kwargs):
        self.name = name
        self.fields = kwargs


def parametrize_test_operation_flow(metafunc) -> None:
    """Parametrize `TestOperation.test_operation_flow`."""
    args_names = ["fields"]
    args_values = []
    args_ids = []

    if hasattr(metafunc.cls, 'flows'):
        flows: List[Flow] = metafunc.cls.flows
        args_values = [[flow.fields] for flow in flows]
        args_ids = [f"flow-{flow.name}" for flow in flows]

    metafunc.parametrize(
        args_names,
        args_values,
        ids=args_ids
    )


class Field:
    """Data required to run `test_operation_field`.

    name: str
    Name of the field in the operation constructor

    text: str
    Title of the field displayed on the screen

    cases: List[Case]
    Every cases to test

    """

    class Case:
        """Data representing a case to test for a given field.

        value: Any
        Value of the field

        name: str
        Identifier for naming the test: <field.name>-<name>

        fields: Dict[str, Any]
        Additionnals fields passed to the operation contstructor

        """

        value: Any
        name: str
        fields: Dict[str, Any]

        def __init__(self, value, name, **kwargs):
            self.value = value
            self.name = name
            self.fields = kwargs

    name: str
    text: str
    cases: List[Case]

    def __init__(self, name, text, cases):
        self.name = name
        self.text = text
        self.cases = cases


def parametrize_test_operation_field(metafunc) -> None:
    """Parametrize `TestOperation.test_operation_field`."""
    args_names = ["field", "case_"]
    args_values = []
    args_ids = []

    if hasattr(metafunc.cls, 'fields'):
        fields_args: List[Field] = metafunc.cls.fields
        args_values = [
            (field, case_)
            for field in fields_args
            for case_ in field.cases
        ]
        args_ids = [
            f"{field.name}-{case_.name}"
            for field in fields_args
            for case_ in field.cases
        ]

    metafunc.parametrize(
        args_names,
        args_values,
        ids=args_ids
    )


def pytest_generate_tests(metafunc) -> None:
    """Need to be include before the `TestOperation` usage."""
    if metafunc.function.__name__ == "test_operation_flow":
        parametrize_test_operation_flow(metafunc)
    if metafunc.function.__name__ == "test_operation_field":
        parametrize_test_operation_field(metafunc)


class TestOperation(ABC):
    """Commun tests for operations."""

    @property
    @abstractmethod
    def op_class(self) -> Type[Op]:
        """Constructor of the Operation class."""
        raise NotImplementedError

    def skip_signature_check(self) -> Optional[str]:
        """Reason why skipping the `test_sign_operation` test."""
        return None

    def test_sign_operation(
            self,
            backend: TezosBackend,
            tezos_navigator: TezosNavigator,
            account: Account
    ):
        """Check signing:
            - Hash
            - Signature
        """
        reason: Optional[str] = self.skip_signature_check()
        if reason is not None:
            pytest.skip(reason)

        message = self.op_class()

        tezos_navigator.toggle_expert_mode()

        with backend.sign(account, message, with_hash=True) as result:
            tezos_navigator.accept_sign()

        account.check_signature(
            message=message,
            with_hash=True,
            data=result.value
        )

    def test_operation_flow(
            self,
            backend: TezosBackend,
            tezos_navigator: TezosNavigator,
            account: Account,
            fields: Dict[str, Any],
            snapshot_dir: Path
    ):
        """Check signing flow

        Will be run for each `Flow` in the field `flows`

        """
        message = self.op_class(**fields)

        tezos_navigator.toggle_expert_mode()

        with backend.sign(account, message):
            tezos_navigator.accept_sign(snap_path=snapshot_dir)

    def test_operation_field(
            self,
            backend: TezosBackend,
            tezos_navigator: TezosNavigator,
            account: Account,
            field: Field,
            case_: Field.Case,
            snapshot_dir: Path
    ):
        """Check how fields are displayed

        Will be run for each `Field.Case` of every `Field` in the
        field `fields`

        """

        fields = case_.fields
        fields[field.name] = case_.value
        operation = self.op_class(**fields)

        tezos_navigator.toggle_expert_mode()

        with backend.sign(account, operation):
            # Navigates until fields
            tezos_navigator.navigate_forward(
                text="Operation",
                validation_instructions=[NavInsID.RIGHT_CLICK],
                screen_change_before_first_instruction=True,
            )

            # Navigates until the field
            tezos_navigator.navigate_forward(
                text=field.text,
                # Even if the screen has changed, we know we are on
                # the right screen because the text has been found
                screen_change_after_last_instruction=False
            )

            # Compare all field's screens
            tezos_navigator.navigate_while_text_and_compare(
                navigate_instruction=NavInsID.RIGHT_CLICK,
                text=field.text,
                snap_path=snapshot_dir,
            )

            # Finish the signing
            tezos_navigator.accept_sign(
                screen_change_before_first_instruction=False
            )
