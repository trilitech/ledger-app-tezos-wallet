/* Tezos Ledger application - Clear signing command handler

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 TriliTech <contact@trili.tech>

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

#include <parser.h>

#include "keys.h"
#include "parser/parser_state.h"

typedef struct {
    cx_blake2b_t state;
    uint8_t      final_hash[SIGN_HASH_SIZE];
} apdu_hash_state_t;

typedef enum {
    SIGN_ST_IDLE,
    SIGN_ST_WAIT_DATA,
    SIGN_ST_WAIT_USER_INPUT
} sign_step_t;

typedef enum {
    BLINDSIGN_ST_OPERATION,
    BLINDSIGN_ST_HASH,
    BLINDSIGN_ST_ACCEPT_REJECT,
} blindsign_step_t;

typedef struct {
    uint8_t packet_index;

    sign_step_t step;
    bool        return_hash;
    bool        received_last_msg;
    uint8_t     tag;

    union {
        struct {
            size_t          total_length;
            tz_parser_state parser_state;
            uint8_t         last_field_index;
            bool            received_msg;
        } clear;
        struct {
            blindsign_step_t step;
        } blind;
    } u;
} apdu_sign_state_t;

#ifdef HAVE_NBGL
void switch_to_blindsigning(const char *, const char *);
#endif
