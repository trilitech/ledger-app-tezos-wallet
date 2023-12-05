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

#include "apdu.h"
#include "app_main.h"
#include "globals.h"

#define CLA 0x80

void
app_exit(void)
{
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
    PRINTF("[SIZEOF] global.stream: %d\n", sizeof(global.stream));
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

static void
dispatch(command_t *cmd)
{
    tz_handler_t f;
    TZ_PREAMBLE(("cmd=0x%p {cla=0x02x ins=%u ...}", cmd, cmd->cla, cmd->ins));

    if (cmd->cla != CLA)
        TZ_FAIL(EXC_CLASS);

    if (tz_ui_stream_get_cb_type() == SCREEN_QUIT) {
        PRINTF("[ERROR] received instruction whilst on Quit screen\n");
        TZ_FAIL(EXC_UNEXPECTED_STATE);
    }

    // clang-format off
    switch (cmd->ins) {
    case INS_VERSION:                   f = handle_apdu_version;        break;
    case INS_SIGN:                      f = handle_apdu_sign;           break;
    case INS_SIGN_WITH_HASH:            f = handle_apdu_sign;           break;
    case INS_PROMPT_PUBLIC_KEY:         f = handle_apdu_get_public_key; break;
    case INS_GET_PUBLIC_KEY:            f = handle_apdu_get_public_key; break;
    case INS_GIT:                       f = handle_apdu_git;            break;
    case INS_AUTHORIZE_BAKING:          f = handle_unimplemented;       break;
    case INS_RESET:                     f = handle_unimplemented;       break;
    case INS_QUERY_AUTH_KEY:            f = handle_unimplemented;       break;
    case INS_QUERY_MAIN_HWM:            f = handle_unimplemented;       break;
    case INS_SETUP:                     f = handle_unimplemented;       break;
    case INS_QUERY_ALL_HWM:             f = handle_unimplemented;       break;
    case INS_QUERY_AUTH_KEY_WITH_CURVE: f = handle_unimplemented;       break;
    case INS_HMAC:                      f = handle_unimplemented;       break;
    case INS_SIGN_UNSAFE:               f = handle_unimplemented;       break;
    default:
        PRINTF("[ERROR] invalid instruction 0x%02x\n", cmd->ins);
        TZ_FAIL(EXC_INVALID_INS);
    }
    // clang-format on

    TZ_CHECK(f(cmd));

    TZ_POSTAMBLE;
}

void
app_main(void)
{
    command_t cmd;
    int       rx;

    app_stack_canary = 0xDEADBEEF;
    FUNC_ENTER(("void"));

    print_memory_layout();
    io_init();
    init_globals();

    /* ST_ERROR implies that we are completely unknown and need to reset */
    global.step = ST_ERROR;

    for (;;) {
        TZ_PREAMBLE(("void"));
        if (global.step == ST_ERROR) {
            global.step = ST_IDLE;
            ui_home_init();
        }

        PRINTF("Ready to receive a command packet.\n");
        rx = io_recv_command();

        if (!apdu_parser(&cmd, G_io_apdu_buffer, rx)) {
            PRINTF("[ERROR] Bad length: %d\n", rx);
            TZ_FAIL(EXC_WRONG_LENGTH_FOR_INS);
        }

        dispatch(&cmd);

        TZ_POSTAMBLE;
    }
}
