/* Tezos Ledger application - Some common primitives and some command handlers

   TODO: split this file (apdu primitives and apdu handlers)

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

#include <string.h>

#include <parser.h>

#include "exception.h"

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

void clear_apdu_globals(void);

/*
 * All of the handlers must be defined together below.  We
 * define them together as they are required to present the
 * same interface and it's easier to validate if they are
 * in the same place.
 *
 * Handlers return a tz_err_t and its value is percolated up
 * and down the stack using the macros defined in exception.h.
 * If the main loop receives a non-zero return, then it should
 * consider that value to be the SW that is returned to the client
 * via io_send_sw().
 *
 * Handlers frequently need to update the global state.  They should do
 * this by directly setting global.step.  Each handler should:
 *
 *     1. if global.step == ST_IDLE, this is first time that the
 *        handler is being called and so all handler specific mem
 *        should be initialised,
 *
 *     2. the handler should assert that it is in the correct state.
 *
 * Note that (1) occurs before (2).  This is because if we are back in
 * ST_IDLE, that means that a previous invocation of this handler or a
 * callback in the UI bits has invalidated any existing transaction that
 * is in flight.  Our variables might be a union with other handlers and
 * all zeroes may not be the default state for any handler and so there
 * is no reliable way for any handler to reset its memory to a factory
 * default setting for another unspecified future handler.  And so, we
 * have each handler reset its data structures up front where we know
 * what the next handler is going to be.
 *
 */

typedef tz_err_t (tz_handler)(command_t *);
typedef tz_handler *tz_handler_t;

tz_handler handle_unimplemented;
tz_handler handle_apdu_version;
tz_handler handle_apdu_git;
tz_handler handle_apdu_get_public_key;
tz_handler handle_apdu_sign;
