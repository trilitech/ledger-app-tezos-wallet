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
#include "apdu.h"
#include "app_main.h"
#include "globals.h"

#define CLA 0x80

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
    PRINTF("[PTR]    G_io_app: 0x%p\n", &G_io_app);
    PRINTF("[SIZEOF] G_io_app: %d\n", sizeof(G_io_app));
}

static void
dispatch(command_t *cmd)
{
    TZ_PREAMBLE(("cmd=0x%p"));

    if (cmd->cla != CLA) {
        TZ_FAIL(EXC_CLASS);
    }

    switch (cmd->ins) {
    case INS_VERSION:
        TZ_CHECK(handle_apdu_version());
        break;
    case INS_GIT:
        TZ_CHECK(handle_apdu_git());
        break;
    case INS_GET_PUBLIC_KEY:
    case INS_PROMPT_PUBLIC_KEY:
        TZ_CHECK(handle_apdu_get_public_key(cmd));
        break;
    case INS_SIGN:
    case INS_SIGN_WITH_HASH:
        TZ_CHECK(handle_apdu_sign(cmd));
        break;
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
