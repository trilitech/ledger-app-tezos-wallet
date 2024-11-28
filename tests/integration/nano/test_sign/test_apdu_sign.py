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

"""Gathering of tests related to Sign instructions."""

from pathlib import Path

from conftest import requires_device
from utils.account import Account
from utils.app import TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import Message, MichelineExpr, Transaction

def test_sign_micheline_without_hash(app: TezosAppScreen, snapshot_dir: Path):
    """Check signing micheline wihout getting hash"""

    message = MichelineExpr([{'string': 'CACA'}, {'string': 'POPO'}, {'string': 'BOUDIN'}])

    with app.backend.sign(DEFAULT_ACCOUNT, message, with_hash=False) as result:
        app.accept_sign(snap_path=snapshot_dir)

    DEFAULT_ACCOUNT.check_signature(
        message=message,
        with_hash=False,
        data=result.value
    )

def test_sign_with_small_packet(app: TezosAppScreen, snapshot_dir: Path):
    """Check signing using small packet instead of full size packets"""

    app.setup_expert_mode()

    def check_sign_with_small_packet(
            account: Account,
            message: Message,
            path: Path) -> None:

        with app.backend.sign(account, message, apdu_size=10) as result:
            app.accept_sign(snap_path=path)

        account.check_signature(
            message=message,
            with_hash=False,
            data=result.value
        )

    message = Transaction(
        source = 'tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu',
        fee = 50000,
        counter = 8,
        gas_limit = 54,
        storage_limit = 45,
        destination = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT',
        amount = 240000,
        entrypoint = 'do',
        parameter = {'prim': 'CAR'}
    )

    check_sign_with_small_packet(
        account=DEFAULT_ACCOUNT,
        message=message,
        path=snapshot_dir)

@requires_device("nanosp")
def test_nanosp_regression_press_right_works_across_apdu_recieves(app: TezosAppScreen, snapshot_dir: Path):
    """Check no need to click right two times between APDUs during signing flow"""

    message = MichelineExpr([{'prim':'IF_NONE','args':[[[{'prim':'SWAP'},{'prim':'IF','args':[[{'prim':'DIP','args':[[[{'prim':'DROP','args':[{'int':1}]},{'prim':'PUSH','args':[{'prim':'unit'},{'prim':'Unit'}]},{'prim':'PUSH','args':[{'prim':'bool'},{'prim':'True'}]},{'prim':'PUSH','args':[{'prim':'string'},{'string':';L\\S?p$-Fq)VDg\n]te\no4v0_8)\"'}]}]]]}],[[{'prim':'DROP','args':[{'int':2}]},{'prim':'PUSH','args':[{'prim':'unit'},{'prim':'Unit'}]},{'prim':'PUSH','args':[{'prim':'bool'},{'prim':'False'}]},{'prim':'PUSH','args':[{'prim':'string'},{'string':'Li-%*edF6~?E[5Kmu?dyviwJ^2\"\\d$FyQ>>!>D$g(Qg'}]},{'prim':'PUSH','args':[{'prim':'string'},{'string':'*Tx<E`SiG6Yf*A^kZ\\=7?H[mOlQ\n]Ehs'}]}]]]}]],[{'prim':'IF_NONE','args':[[{'prim':'DUP'}],[[{'prim':'DROP','args':[{'int':4}]},{'prim':'PUSH','args':[{'prim':'unit'},{'prim':'Unit'}]},{'prim':'PUSH','args':[{'prim':'bool'},{'prim':'True'}]},{'prim':'PUSH','args':[{'prim':'string'},{'string':'\"\\6_4\n$k%'}]},{'prim':'PUSH','args':[{'prim':'string'},{'string':'c^1\"\\?Ey_1!EVb~9;EX;YU\n#Kj2ZT8h`U!X '}]}]]]}]]},{'prim':'SIZE'}])

    with app.backend.sign(DEFAULT_ACCOUNT, message, with_hash=True) as result:
        app.accept_sign(snap_path=snapshot_dir)

    DEFAULT_ACCOUNT.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )
