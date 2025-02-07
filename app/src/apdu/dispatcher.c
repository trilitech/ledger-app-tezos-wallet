/* Tezos Ledger application - Instruction dispatcher

   Copyright 2025 Functori <contact@functori.com>
   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Trilitech <contact@trili.tech>

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

#include <stdbool.h>

#include <buffer.h>
#include <os_io_seproxyhal.h>  // G_io_apdu_media, IO_APDU_MEDIA_U2F
#include <parser.h>

#include "dispatcher.h"

#include "compat.h"
#include "exception.h"
#include "globals.h"
#include "keys.h"

#include "get_git_commit.h"
#include "get_pubkey.h"
#include "get_version.h"
#include "sign.h"

#define CLA 0x80  /// The only APDU class that will be used

// Instruction codes
#define INS_VERSION           0x00
#define INS_GET_PUBLIC_KEY    0x02
#define INS_PROMPT_PUBLIC_KEY 0x03
#define INS_SIGN              0x04
#define INS_GIT               0x09
#define INS_SIGN_WITH_HASH    0x0F

/// Packet indexes
#define P1_FIRST       0x00u  /// First packet
#define P1_NEXT        0x01u  /// Other packet
#define P1_LAST_MARKER 0x80u  /// Last packet

/// Parameters parser helpers
#define ASSERT_GLOBAL_STEP(_step) \
    TZ_ASSERT(EXC_UNEXPECTED_STATE, global.step == (_step))
#define ASSERT_NO_P1(_cmd) TZ_ASSERT(EXC_WRONG_PARAM, _cmd->p1 == 0u)
#define ASSERT_NO_P2(_cmd) TZ_ASSERT(EXC_WRONG_PARAM, _cmd->p2 == 0u)
#define READ_P2_DERIVATION_TYPE(_cmd, _derivation_type)               \
    derivation_type_t _derivation_type = (derivation_type_t)_cmd->p2; \
    TZ_ASSERT(EXC_WRONG_PARAM, DERIVATION_TYPE_IS_SET(_derivation_type))
#define ASSERT_NO_DATA(_cmd) TZ_ASSERT(EXC_WRONG_VALUES, _cmd->data == NULL)
#define READ_DATA(_cmd, _buf) \
    buffer_t _buf = {         \
        .ptr    = _cmd->data, \
        .size   = _cmd->lc,   \
        .offset = 0u,         \
    }

/**
 * @brief Read the APDU of the signing command and choose the next action to
 * take.
 *
 * Same as dispatch but focus on signing instructions.
 *
 * @param cmd: command containg APDU received
 */
static void
dispatch_sign_instruction(const command_t *cmd)
{
    TZ_PREAMBLE(("cmd=0x%p"));

    TZ_ASSERT(EXC_UNEXPECTED_STATE,
              (cmd->ins == INS_SIGN_WITH_HASH) || (cmd->ins == INS_SIGN));

    bool return_hash = cmd->ins == INS_SIGN_WITH_HASH;

    if ((cmd->p1 & ~P1_LAST_MARKER) == P1_FIRST) {
        TZ_ASSERT(EXC_UNEXPECTED_STATE,
                  (global.step == ST_IDLE) || (global.step == ST_SWAP_SIGN));

        READ_P2_DERIVATION_TYPE(cmd, derivation_type);
        READ_DATA(cmd, buf);

        TZ_CHECK(
            handle_signing_key_setup(&buf, derivation_type, return_hash));
    } else {
        TZ_ASSERT(EXC_UNEXPECTED_STATE,
                  (global.step == ST_BLIND_SIGN)
                      || (global.step == ST_CLEAR_SIGN)
                      || (global.step == ST_SUMMARY_SIGN)
                      || (global.step == ST_SWAP_SIGN));

        bool last = (cmd->p1 & P1_LAST_MARKER) != 0;

        READ_DATA(cmd, buf);

        TZ_CHECK(handle_sign(&buf, last, return_hash));
    }

    TZ_POSTAMBLE;
}

void
dispatch(const command_t *cmd)
{
    TZ_PREAMBLE(("cmd=0x%p"));

    if (cmd->cla != CLA) {
        TZ_FAIL(EXC_CLASS);
    }

    switch (cmd->ins) {
    case INS_VERSION:

        ASSERT_GLOBAL_STEP(ST_IDLE);

        handle_get_version();

        break;
    case INS_GIT:

        ASSERT_GLOBAL_STEP(ST_IDLE);

        handle_get_git_commit();

        break;
    case INS_GET_PUBLIC_KEY:
    case INS_PROMPT_PUBLIC_KEY: {
        TZ_ASSERT(EXC_UNEXPECTED_STATE,
                  (global.step == ST_IDLE) || (global.step == ST_SWAP_SIGN));

        ASSERT_NO_P1(cmd);
        READ_P2_DERIVATION_TYPE(cmd, derivation_type);
        READ_DATA(cmd, buf);

        bool prompt = cmd->ins == INS_PROMPT_PUBLIC_KEY;

        // do not expose pks without prompt through U2F (permissionless legacy
        // comm in browser)
        TZ_ASSERT(EXC_HID_REQUIRED,
                  prompt || (G_io_apdu_media != IO_APDU_MEDIA_U2F));

        TZ_CHECK(handle_get_public_key(&buf, derivation_type, prompt));

        break;
    }
    case INS_SIGN:
    case INS_SIGN_WITH_HASH: {
        TZ_CHECK(dispatch_sign_instruction(cmd));
        break;
    }
    default:
        PRINTF("[ERROR] invalid instruction 0x%02x\n", cmd->ins);
        TZ_FAIL(EXC_INVALID_INS);
    }

    TZ_POSTAMBLE;
}
