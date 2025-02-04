/* Tezos Ledger application - Application main loop

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>
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

#include <os.h>
#include <cx.h>
#include <io.h>
#include <parser.h>
#ifdef HAVE_SWAP
#include <swap.h>
#endif

#include "app_main.h"

#include "apdu.h"
#include "globals.h"
#include "keys.h"

void
app_exit(void)
{
    PRINTF("[DEBUG] Trying to exit the app. \n");
    os_sched_exit(-1);
}

static void
print_memory_layout(void)
{
    PRINTF("[PTR]    stack canary: 0x%x\n", &app_stack_canary);
    PRINTF("[PTR]    G_io_apdu_buffer: 0x%p\n", G_io_apdu_buffer);
    PRINTF("[PTR]    global: 0x%p\n", &global);
    PRINTF("[SIZEOF] global: %d\n", sizeof(global));
    PRINTF("[SIZEOF] global.keys.apdu.sign: %d\n",
           sizeof(global.keys.apdu.sign));
    PRINTF("[SIZEOF] global.keys.apdu.hash: %d\n",
           sizeof(global.keys.apdu.hash));
    PRINTF("[SIZEOF] global.ui.stream: %d\n", sizeof(global.ui.stream));
    PRINTF("[PTR]    G_io_apdu_buffer: 0x%p\n", G_io_apdu_buffer);
    PRINTF("[SIZEOF] G_io_apdu_buffer: %u\n", sizeof(G_io_apdu_buffer));
    PRINTF("[PTR]    G_io_seproxyhal_spi_buffer: 0x%p\n",
           G_io_seproxyhal_spi_buffer);
    PRINTF("[SIZEOF] G_io_seproxyhal_spi_buffer: %u\n",
           sizeof(G_io_seproxyhal_spi_buffer));
    PRINTF("[PTR]    G_ux: 0x%p\n", &G_ux);
    PRINTF("[SIZEOF] G_ux: %u\n", sizeof(G_ux));
    PRINTF("[PTR]    G_ux_os: 0x%p\n", &G_ux_os);
    PRINTF("[SIZEOF] G_ux_os: %u\n", sizeof(G_ux_os));
    PRINTF("[PTR]    G_ux_params: 0x%p\n", &G_ux_params);
    PRINTF("[SIZEOF] G_ux_params: %u\n", sizeof(G_ux_params));
    PRINTF("[PTR]    G_io_usb_ep_buffer: 0x%p\n", G_io_usb_ep_buffer);
    PRINTF("[SIZEOF] G_io_usb_ep_buffer: %d\n", sizeof(G_io_usb_ep_buffer));
    PRINTF("[PTR]    G_io_app: 0x%p\n", &G_io_app);
    PRINTF("[SIZEOF] G_io_app: %d\n", sizeof(G_io_app));
}

#define CLA 0x80  /// The only APDU class that will be used

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

/**
 * @brief Read the APDU of the command and choose the next action to take.
 *
 * The APDU structure is defined by [ISO/IEC
 * 7816-4](https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit).
 * This function ensures that the command complies with the [APDU
 * specification for our application](app/docs/apdu.md) and, depending on the
 * code instruction, parse the instruction parameters in order to supply them,
 * in addition to the potential command data, to the corresponding process.
 *
 * @param cmd: command containg APDU received
 */
static void
dispatch(const command_t *cmd)
{
    TZ_PREAMBLE(("cmd=0x%p"));

    if (cmd->cla != CLA) {
        TZ_FAIL(EXC_CLASS);
    }

    switch (cmd->ins) {
    case INS_VERSION:

        ASSERT_GLOBAL_STEP(ST_IDLE);

        handle_version();

        break;
    case INS_GIT:

        ASSERT_GLOBAL_STEP(ST_IDLE);

        handle_git();

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

void
app_main(void)
{
    command_t cmd;
    int       input_len = 0;

    app_stack_canary = 0xDEADBEEFu;
    FUNC_ENTER(("void"));

    print_memory_layout();
    io_init();
    init_globals();

    /* ST_ERROR implies that we are completely unknown and need to reset */
    global.step = ST_ERROR;
    for (;;) {
        TZ_PREAMBLE(("void"));
#ifdef HAVE_SWAP
        if (G_called_from_swap) {
            global.step = ST_SWAP_SIGN;
        }
        PRINTF("[SWAP] : G_called_from_swap = %d , global.step = %d",
               G_called_from_swap, global.step);
#endif
        if (global.step == ST_ERROR) {
            global.step = ST_IDLE;
            ui_home_init();
        }

        PRINTF("Ready to receive a command packet.\n");
        input_len = io_recv_command();
        if (input_len < 0) {
            PRINTF("=> io_recv_command failure\n");
            return;
        }

        if (!apdu_parser(&cmd, G_io_apdu_buffer, input_len)) {
            PRINTF("[ERROR] Bad length: %d\n", input_len);
            TZ_FAIL(EXC_WRONG_LENGTH_FOR_INS);
        }

        PRINTF(
            "=> CLA=%02X | INS=%02X | P1=%02X | P2=%02X | Lc=%02X | "
            "CData=%.*H\n",
            cmd.cla, cmd.ins, cmd.p1, cmd.p2, cmd.lc, cmd.lc, cmd.data);

        TZ_CHECK(dispatch(&cmd));

        TZ_POSTAMBLE;
    }
}
