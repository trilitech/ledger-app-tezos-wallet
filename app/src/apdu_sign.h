/* Tezos Ledger application - Clear signing command handler

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

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

#include <unistd.h>
#include <stdbool.h>

#include "keys.h"
#include "parser/parser_state.h"

typedef struct {
    cx_blake2b_t state;
    uint8_t final_hash[SIGN_HASH_SIZE];
} apdu_hash_state_t;

typedef enum {
    SIGN_ST_IDLE,
    SIGN_ST_WAIT_DATA,
    SIGN_ST_WAIT_USER_INPUT
} sign_step_t;

typedef struct {
    uint8_t packet_index;

    sign_step_t step;
    bool return_hash;
    bool received_last_msg;

    union {
        struct {
            size_t total_length;
            tz_parser_state parser_state;
        } clear;
        struct {
            uint8_t tag;
        } blind;
    } u;
} apdu_sign_state_t;

/* Macros to assert our state */

#define APDU_SIGN_ASSERT(_cond)  TZ_ASSERT(EXC_UNEXPECTED_SIGN_STATE, (_cond))
#define APDU_SIGN_ASSERT_STEP(x) APDU_SIGN_ASSERT(global.apdu.sign.step == (x))

/* Prototypes */

size_t handle_apdu_sign(bool);
