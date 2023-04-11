/* Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

   With code excerpts from:
    - Legacy Tezos app, Copyright 2019 Obsidian Systems
    - Ledger Blue sample apps, Copyright 2016 Ledger

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#pragma once

#include <string.h>

#define OFFSET_CLA   0
#define OFFSET_INS   1  // instruction code
#define OFFSET_P1    2  // user-defined 1-byte parameter
#define OFFSET_P2    3
#define OFFSET_CURVE OFFSET_P2
#define OFFSET_LC    4  // length of CDATA
#define OFFSET_CDATA 5  // payload

// Instruction codes
#define INS_VERSION                   0x00
#define INS_AUTHORIZE_BAKING          0x01
#define INS_GET_PUBLIC_KEY            0x02
#define INS_PROMPT_PUBLIC_KEY         0x03
#define INS_SIGN                      0x04
#define INS_SIGN_UNSAFE               0x05  // Data that is already hashed.
#define INS_RESET                     0x06
#define INS_QUERY_AUTH_KEY            0x07
#define INS_QUERY_MAIN_HWM            0x08
#define INS_GIT                       0x09
#define INS_SETUP                     0x0A
#define INS_QUERY_ALL_HWM             0x0B
#define INS_DEAUTHORIZE               0x0C
#define INS_QUERY_AUTH_KEY_WITH_CURVE 0x0D
#define INS_HMAC                      0x0E
#define INS_SIGN_WITH_HASH            0x0F

// Last valid instruction code
#define INS_MAX                       0x0F

size_t finalize_successful_send(size_t tx);
void delayed_send(size_t tx);
void delay_reject(void);
void require_permissioned_comm(void);

size_t handle_apdu_version(void);
size_t handle_apdu_git(void);
