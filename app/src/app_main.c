/* Tezos Ledger application - Application main loop

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

#include "app_main.h"

#include "os.h"
#include "cx.h"
#include "io.h"

#include "globals.h"

#include "apdu.h"

#include "apdu_sign.h"
#include "apdu_pubkey.h"

void app_exit(void) {
  os_sched_exit(-1);
}

static uint8_t dispatch(uint8_t instruction) {
  FUNC_ENTER(("%u", instruction));

  switch (tz_ui_stream_get_type()) {
  case SCREEN_QUIT:
    PRINTF("[ERROR] received instruction whilst on Quit screen\n");
    THROW(EXC_UNEXPECTED_STATE);
  default:
    break;
  }

  switch (instruction) {
  case INS_SIGN:
  case INS_SIGN_WITH_HASH:
      return handle_apdu_sign(instruction == INS_SIGN_WITH_HASH);
  case INS_VERSION:
    if (!(global.step == ST_IDLE)) THROW(EXC_UNEXPECTED_STATE);
    return handle_apdu_version();
  case INS_PROMPT_PUBLIC_KEY:
  case INS_GET_PUBLIC_KEY:
    if (!(global.step == ST_IDLE)) THROW(EXC_UNEXPECTED_STATE);
    return handle_apdu_get_public_key(instruction == INS_PROMPT_PUBLIC_KEY);
  case INS_GIT:
    if (!(global.step == ST_IDLE)) THROW(EXC_UNEXPECTED_STATE);
    return handle_apdu_git();
  case INS_AUTHORIZE_BAKING:
  case INS_RESET:
  case INS_QUERY_AUTH_KEY:
  case INS_QUERY_MAIN_HWM:
  case INS_SETUP:
  case INS_QUERY_ALL_HWM:
  case INS_QUERY_AUTH_KEY_WITH_CURVE:
  case INS_HMAC:
  case INS_SIGN_UNSAFE:
  default:
    PRINTF("[ERROR] invalid instruction %02X\n", instruction);
    THROW(EXC_INVALID_INS);
  }
  FUNC_LEAVE();
}


#define CLA 0x80

void app_main() {
    FUNC_ENTER(("void"));
    app_stack_canary = 0xDEADBEEF;

    io_init();
    init_globals();

    PRINTF("[PTR]    stack canary: 0x%x\n", &app_stack_canary);
    PRINTF("[PTR]    G_io_apdu_buffer: 0x%p\n", G_io_apdu_buffer);
    PRINTF("[PTR]    global: 0x%p\n", &global);
    PRINTF("[SIZEOF] global: %d\n", sizeof(global));
    PRINTF("[SIZEOF] global.apdu.sign: %d\n", sizeof(global.apdu.sign));
    PRINTF("[SIZEOF] global.apdu.hash: %d\n", sizeof(global.apdu.hash));
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

    global.step = ST_IDLE;
    ui_home_init();

    volatile size_t rx = io_exchange(CHANNEL_APDU, 0);

    while (true) {

        BEGIN_TRY {
            TRY {
                PRINTF("[DEBUG] recv(0x%.*H)\n", rx, G_io_apdu_buffer);
                // Process APDU of size rx

                if (rx == 0) {
                    // no apdu received, well, reset the session, and reset the
                    // bootloader configuration
                    THROW(EXC_SECURITY);
                }

                if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                    THROW(EXC_CLASS);
                }

                // The amount of bytes we get in our APDU must match what
                // the APDU declares its own content length is. All these
                // values are unsigned, so this implies that
                // if rx < OFFSET_CDATA it also throws.
                if (rx != G_io_apdu_buffer[OFFSET_LC] + OFFSET_CDATA) {
                    THROW(EXC_WRONG_LENGTH);
                }

                uint8_t const instruction = G_io_apdu_buffer[OFFSET_INS];
                size_t const tx = dispatch(instruction);

                rx = io_exchange(CHANNEL_APDU, tx);
            }
            CATCH(ASYNC_EXCEPTION) {
                rx = io_exchange(CHANNEL_APDU | IO_ASYNCH_REPLY, 0);
            }
            CATCH(EXCEPTION_IO_RESET) {
                THROW(EXCEPTION_IO_RESET);
            }
            CATCH_OTHER(e) {
                clear_apdu_globals();  // IMPORTANT: Application state must
                                       // not persist through errors
                global.step = ST_IDLE;

                uint16_t sw = e;
                PRINTF("[ERROR] caught at top level, number: %x\n", sw);
                switch (sw) {
                    default:
                        sw = 0x6800 | (e & 0x7FF);
                        __attribute__((fallthrough));
                    case 0x6000 ... 0x6FFF:
                        __attribute__((fallthrough));
                    case 0x9000 ... 0x9FFF: {
                        PRINTF("[ERROR] line number: %d\n", sw & 0x0FFF);
                        size_t tx = 0;
                        G_io_apdu_buffer[tx++] = sw >> 8;
                        G_io_apdu_buffer[tx++] = sw;
                        rx = io_exchange(CHANNEL_APDU, tx);
                        break;
                    }
                }
            }
            FINALLY {
            }
        }
        END_TRY;
    }
}
