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
from typing import Any, Dict, List, Type, TypeVar

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


def pytest_generate_tests(metafunc) -> None:
    """Need to be include before the `TestOperation` usage."""
    if metafunc.function.__name__ == "test_operation_flow":
        parametrize_test_operation_flow(metafunc)


class TestOperation(ABC):
    """Commun tests for operations."""

    @property
    @abstractmethod
    def op_class(self) -> Type[Op]:
        """Constructor of the Operation class."""
        raise NotImplementedError

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
