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

import os
import sys

file_path=os.path.abspath(__file__)
dir_path=os.path.dirname(file_path)
root_path=os.path.dirname(dir_path)
sys.path.append(root_path)

from pathlib import Path

from utils.app import nano_app, Screen, DEFAULT_ACCOUNT
from utils.message import Message

# Expression: {IF_NONE {{SWAP;IF {DIP {{DROP 1;PUSH unit Unit;PUSH bool True;PUSH string ";L\\S?p$-Fq)VDg\n]te\no4v0_8)\""}}} {{DROP 2;PUSH unit Unit;PUSH bool False;PUSH string "Li-%*edF6~?E[5Kmu?dyviwJ^2\"\\d$FyQ>>!>D$g(Qg";PUSH string "*Tx<E`SiG6Yf*A^kZ\\=7?H[mOlQ\n]Ehs"}}}} {IF_NONE {DUP} {{DROP 4;PUSH unit Unit;PUSH bool True;PUSH string "\"\\6_4\n$k%";PUSH string "c^1\"\\?Ey_1!EVb~9;EX;YU\n#Kj2ZT8h`U!X "}}};SIZE}

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        message = Message.from_bytes("050200000135072f02000000c502000000c0034c072c0200000040051f02000000390200000034052000010743036c030b07430359030a07430368010000001b3b4c5c533f70242d4671295644670a5d74650a6f3476305f3829220200000072020000006d052000020743036c030b07430359030307430368010000002b4c692d252a656446367e3f455b354b6d753f64797669774a5e32225c64244679513e3e213e4424672851670743036801000000202a54783c45605369473659662a415e6b5a5c3d373f485b6d4f6c510a5d4568730200000062072f020000000203210200000054020000004f052000040743036c030b07430359030a074303680100000009225c365f340a246b25074303680100000024635e31225c3f45795f31214556627e393b45583b59550a234b6a325a54386860552158200345")

        data = app.sign(DEFAULT_ACCOUNT,
                        message,
                        with_hash=True,
                        path=test_name)

        app.check_signature(
            account=DEFAULT_ACCOUNT,
            message=message,
            with_hash=True,
            data=data)

        app.quit()
