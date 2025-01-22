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

"""Tezos app backend."""

from contextlib import contextmanager
from enum import IntEnum
from multiprocessing.pool import ThreadPool
from struct import unpack
import time
from typing import Callable, Generator, TypeVar, Union

from types import SimpleNamespace

from ragger.backend import SpeculosBackend
from ragger.backend.interface import BackendInterface, RAPDU
from ragger.error import ExceptionRAPDU

from .account import Account, SigType
from .message import Message


class Version:
    """Class representing the version."""

    class AppKind(IntEnum):
        """Class representing the kind of app."""

        WALLET = 0x00
        BAKING = 0x01

        def __str__(self) -> str:
            return self.name

    app_kind: AppKind
    major: int
    minor: int
    patch: int

    def __init__(self,
                 app_kind: AppKind,
                 major: int,
                 minor: int,
                 patch: int):
        self.app_kind = app_kind
        self.major = major
        self.minor = minor
        self.patch = patch

    def __repr__(self) -> str:
        return f"App {self.app_kind}: {self.major}.{self.minor}.{self.patch}"

    def __eq__(self, other: object):
        if not isinstance(other, Version):
            return NotImplemented
        return \
            self.app_kind == other.app_kind and \
            self.major == other.major and \
            self.minor == other.minor and \
            self.patch == other.patch

    @classmethod
    def from_bytes(cls, raw: bytes) -> 'Version':
        """Create a version from bytes."""
        (app_kind, major, minor, patch) = unpack('<bbbb', raw)
        return Version(Version.AppKind(app_kind), major, minor, patch)


class Cla(IntEnum):
    """Class representing APDU class."""

    DEFAULT = 0x80

    def __str__(self) -> str:
        return self.name

class Ins(IntEnum):
    """Class representing instruction."""

    VERSION                   = 0x00
    AUTHORIZE_BAKING          = 0x01
    GET_PUBLIC_KEY            = 0x02
    PROMPT_PUBLIC_KEY         = 0x03
    SIGN                      = 0x04
    SIGN_UNSAFE               = 0x05
    RESET                     = 0x06
    QUERY_AUTH_KEY            = 0x07
    QUERY_MAIN_HWM            = 0x08
    GIT                       = 0x09
    SETUP                     = 0x0a
    QUERY_ALL_HWM             = 0x0b
    DEAUTHORIZE               = 0x0c
    QUERY_AUTH_KEY_WITH_CURVE = 0x0d
    HMAC                      = 0x0e
    SIGN_WITH_HASH            = 0x0f

    def __str__(self) -> str:
        return self.name

class Index(IntEnum):
    """Class representing packet index."""

    FIRST      = 0x00
    OTHER      = 0x01
    LAST       = 0x80
    OTHER_LAST = 0x81

    def __str__(self) -> str:
        return self.name

class StatusCode(IntEnum):
    """Class representing the status code."""

    SECURITY                  = 0x6982
    HID_REQUIRED              = 0x6983
    REJECT                    = 0x6985
    WRONG_VALUES              = 0x6A80
    REFERENCED_DATA_NOT_FOUND = 0x6A88
    WRONG_PARAM               = 0x6B00
    WRONG_LENGTH              = 0x6C00
    INVALID_INS               = 0x6D00
    CLASS                     = 0x6E00
    OK                        = 0x9000
    UNEXPECTED_STATE          = 0x9001
    UNEXPECTED_SIGN_STATE     = 0x9002
    UNKNOWN_CX_ERR            = 0x9003
    UNKNOWN                   = 0x90FF
    WRONG_LENGTH_FOR_INS      = 0x917E
    MEMORY_ERROR              = 0x9200
    PARSE_ERROR               = 0x9405

    def __str__(self) -> str:
        return self.name

    @contextmanager
    def expected(self) -> Generator[None, None, None]:
        """Fail if the right RAPDU code exception is not raise."""
        try:
            yield
            assert self == StatusCode.OK, \
                f"Expect fail with {self} but succeed"
        except ExceptionRAPDU as e:
            try:
                status = f"{StatusCode(e.status)}"
            except ValueError:
                status = f"0x{e.status:x}"
            assert self == e.status, \
                f"Expect fail with {self} but fail with {status}"


RESPONSE = TypeVar('RESPONSE')


def async_thread(function: Callable[..., RESPONSE]):
    """Decorator that runs asynchronously the function and provides a
    context manager to access the result in a `value` field"""
    @contextmanager
    def wrapper(*args, **kwargs) -> Generator[SimpleNamespace, None, None]:
        thread_result = SimpleNamespace(value=None)
        thread_pool = ThreadPool(processes=1)
        try:
            thread_async_result = thread_pool.apply_async(
                function,
                args=args,
                kwds=kwargs
            )
            yield thread_result
            thread_result.value = thread_async_result.get()
        finally:
            thread_pool.terminate()
    return wrapper


MAX_APDU_SIZE: int = 235

class TezosBackend(BackendInterface):
    """Class representing the backen of the tezos app."""

    def _exchange(self,
                  ins: Union[Ins, int],
                  index: Union[Index, int] = Index.FIRST,
                  sig_type: Union[SigType, int, None] = None,
                  payload: bytes = b'') -> bytes:
        """Override of `exchange` for the tezos app."""

        assert len(payload) <= MAX_APDU_SIZE, f"Apdu too large {len(payload)}"

        # Set to a non-existent value to ensure that p2 is unused
        p2: int = sig_type if sig_type is not None else 0xff

        rapdu: RAPDU = self.exchange(Cla.DEFAULT,
                                     ins,
                                     p1=index,
                                     p2=p2,
                                     data=payload)

        if rapdu.status != StatusCode.OK:
            raise ExceptionRAPDU(rapdu.status, rapdu.data)

        return rapdu.data

    def git(self) -> bytes:
        """Requests the app commit."""
        return self._exchange(Ins.GIT)

    def version(self) -> bytes:
        """Requests the app version."""
        return self._exchange(Ins.VERSION)

    def _provide_public_key(self,
                            account: Account,
                            with_prompt: bool = False) -> bytes:
        """Requests the public key according to the account.
        Use `with_prompt` ask user confirmation
        """
        ins = Ins.PROMPT_PUBLIC_KEY if with_prompt else Ins.GET_PUBLIC_KEY
        return self._exchange(ins,
                              sig_type=account.sig_type,
                              payload=account.path)

    def get_public_key(self, account: Account) -> bytes:
        """Requests the public key according to the account."""
        return self._provide_public_key(account, with_prompt=False)

    @async_thread
    def prompt_public_key(self, account: Account) -> bytes:
        """Requests the public key according to the account.  Ask for
        a user confirmation"""
        return self._provide_public_key(account, with_prompt=True)

    def _ask_sign(self, ins: Ins, account: Account) -> None:
        """Prepare to sign with the account."""
        data: bytes = self._exchange(ins, sig_type=account.sig_type, payload=account.path)
        assert not data, f"No data expected but got {data.hex()}"

    def _continue_sign(self, ins: Ins, payload: bytes, last: bool) -> bytes:
        """Sends payload to sign.
        Use `last` when sending the last packet
        """
        index: Index = Index.OTHER
        if last:
            index = Index(index | Index.LAST)
        return self._exchange(ins, index, payload=payload)

    @async_thread
    def sign(self,
             account: Account,
             message: Message,
             with_hash: bool = False,
             apdu_size: int = MAX_APDU_SIZE) -> bytes:
        """Requests the signature of a message."""
        msg = bytes(message)
        assert msg, "Do not sign empty message"

        ins = Ins.SIGN_WITH_HASH if with_hash else Ins.SIGN

        self._ask_sign(ins, account)

        while msg:
            payload = msg[:apdu_size]
            msg     = msg[apdu_size:]
            last    = not msg
            data    = self._continue_sign(ins, payload, last)
            if last:
                return data
            assert not data, f"No data expected but got {data.hex()}"

        assert False, "We should have already returned"

MAX_ATTEMPTS = 50

def with_retry(f, attempts=MAX_ATTEMPTS):
    """Try f until it succeeds or has been executed attempts times."""
    while True:
        try:
            return f()
        except Exception as e:
            if attempts <= 0:
                print("- with_retry: attempts exhausted -")
                raise e
        attempts -= 1
        # Give plenty of time for speculos to update - can take a long time on CI machines
        time.sleep(0.5)

class SpeculosTezosBackend(TezosBackend, SpeculosBackend):
    """Class representing Tezos app running on Speculos."""

    # speculos can be slow to start up in a slow environment.
    # Here, we expect a little more
    def __enter__(self) -> "SpeculosTezosBackend":
        try:
            super().__enter__()
            return self
        except Exception:
            process = self._client.process
            try:
                with_retry(self._client._wait_until_ready, attempts=5)
                super().__enter__()
            except Exception as e:
                self._client.stop()
                # do not forget to close the first process
                self._client.process = process
                self._client.stop()
                raise e
            # replace the new process by the first one
            self._client.process.kill()
            self._client.process.wait()
            self._client.process = process
            return self
