#!/usr/bin/env python3
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

"""Check no need to click right two times between APDUs during signing flow"""

from pathlib import Path

from conftest import requires_device

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import MichelineExpr

@requires_device("nanosp")
def test_nanosp_regression_press_right_works_across_apdu_recieves(app: TezosAppScreen):
    """Check no need to click right two times between APDUs during signing flow"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    message = MichelineExpr([{'prim':'IF_NONE','args':[[[{'prim':'SWAP'},{'prim':'IF','args':[[{'prim':'DIP','args':[[[{'prim':'DROP','args':[{'int':1}]},{'prim':'PUSH','args':[{'prim':'unit'},{'prim':'Unit'}]},{'prim':'PUSH','args':[{'prim':'bool'},{'prim':'True'}]},{'prim':'PUSH','args':[{'prim':'string'},{'string':';L\\S?p$-Fq)VDg\n]te\no4v0_8)\"'}]}]]]}],[[{'prim':'DROP','args':[{'int':2}]},{'prim':'PUSH','args':[{'prim':'unit'},{'prim':'Unit'}]},{'prim':'PUSH','args':[{'prim':'bool'},{'prim':'False'}]},{'prim':'PUSH','args':[{'prim':'string'},{'string':'Li-%*edF6~?E[5Kmu?dyviwJ^2\"\\d$FyQ>>!>D$g(Qg'}]},{'prim':'PUSH','args':[{'prim':'string'},{'string':'*Tx<E`SiG6Yf*A^kZ\\=7?H[mOlQ\n]Ehs'}]}]]]}]],[{'prim':'IF_NONE','args':[[{'prim':'DUP'}],[[{'prim':'DROP','args':[{'int':4}]},{'prim':'PUSH','args':[{'prim':'unit'},{'prim':'Unit'}]},{'prim':'PUSH','args':[{'prim':'bool'},{'prim':'True'}]},{'prim':'PUSH','args':[{'prim':'string'},{'string':'\"\\6_4\n$k%'}]},{'prim':'PUSH','args':[{'prim':'string'},{'string':'c^1\"\\?Ey_1!EVb~9;EX;YU\n#Kj2ZT8h`U!X '}]}]]]}]]},{'prim':'SIZE'}])

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=test_name)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()
