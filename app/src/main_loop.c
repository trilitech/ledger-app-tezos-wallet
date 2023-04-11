/* Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include "main_loop.h"

#include "os.h"
#include "cx.h"
#include "ui.h"

#include "globals.h"

#include "apdu.h"

#include "apdu_sign.h"
#include "apdu_pubkey.h"

static uint8_t dispatch(uint8_t instruction) {
  switch (instruction) {
  case INS_SIGN:
  case INS_SIGN_WITH_HASH: {
    if (!(global.step == ST_IDLE || global.step == ST_SIGN)) THROW (EXC_UNEXPECTED_STATE);
    return handle_apdu_sign(instruction == INS_SIGN_WITH_HASH);
  }
  case INS_VERSION: {
    if (!(global.step == ST_IDLE)) THROW (EXC_UNEXPECTED_STATE);
    return handle_apdu_version();
  }
  case INS_PROMPT_PUBLIC_KEY:
  case INS_GET_PUBLIC_KEY: {
    if (!(global.step == ST_IDLE)) THROW (EXC_UNEXPECTED_STATE);
    return handle_apdu_get_public_key(instruction == INS_PROMPT_PUBLIC_KEY);
  }
  case INS_GIT: {
    if (!(global.step == ST_IDLE)) THROW (EXC_UNEXPECTED_STATE);
    return handle_apdu_git();
  }
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
}


#define CLA 0x80

__attribute__((noreturn)) void main_loop() {
    global.step = ST_IDLE;

    PRINTF("[SIZEOF] global: %d\n", sizeof(global));
    PRINTF("[SIZEOF] global.apdu.sign: %d\n", sizeof(global.apdu.sign));
    PRINTF("[SIZEOF] global.apdu.hash: %d\n", sizeof(global.apdu.hash));
    PRINTF("[SIZEOF] global.stream: %d\n", sizeof(global.stream));
    PRINTF("[SIZEOF] global.ux: %d\n", sizeof(global.ux));

    ui_initial_screen();

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

                // The amount of bytes we get in our APDU must match what the APDU declares
                // its own content length is. All these values are unsigned, so this implies
                // that if rx < OFFSET_CDATA it also throws.
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
                clear_apdu_globals();  // IMPORTANT: Application state must not persist through
                                       // errors
                global.step = ST_IDLE;

                uint16_t sw = e;
                PRINTF("[ERROR] caught at top level, number: %x\n", sw);
                switch (sw) {
                    default:
                        sw = 0x6800 | (e & 0x7FF);
                        // FALL THROUGH
                    case 0x6000 ... 0x6FFF:
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

// I have no idea what this function does, but it is called by the OS
unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
        case CHANNEL_KEYBOARD:
            break;

        // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
        case CHANNEL_SPI:
            if (tx_len) {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

                if (channel & IO_RESET_AFTER_REPLIED) {
                    reset();
                }
                return 0;  // nothing received from the master so far (it's a tx
                           // transaction)
            } else {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
            }

        default:
            THROW(INVALID_PARAMETER);
    }
    return 0;
}
