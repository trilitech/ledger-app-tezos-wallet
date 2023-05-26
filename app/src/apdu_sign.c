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

#include "cx.h"

#include "apdu.h"
#include "apdu_sign.h"
#include "globals.h"
#include "keys.h"
#include "memory.h"
#include "ui_stream.h"

#include "parser/parser_state.h"
#include "parser/operation_parser.h"

#include <string.h>
#include <stdbool.h>

#define P1_FIRST          0x00
#define P1_NEXT           0x01
#define P1_HASH_ONLY_NEXT 0x03  // You only need it once
#define P1_LAST_MARKER    0x80

static inline void clear_data(void) {
    memset(&global.apdu.hash, 0, sizeof(global.apdu.hash));
    memset(&global.apdu.sign, 0, sizeof(global.apdu.sign));
}

typedef struct {
  uint8_t cla;
  uint8_t ins;
  uint8_t p1;
  uint8_t p2;
  uint8_t buff_size;
  uint8_t *buff;
  bool is_last;
  bool is_first;
} packet_t;

static void init_packet(packet_t *pkt) {
  pkt->cla = G_io_apdu_buffer[OFFSET_CLA];
  pkt->ins = G_io_apdu_buffer[OFFSET_INS];
  pkt->p1 = G_io_apdu_buffer[OFFSET_P1];
  pkt->p2 = G_io_apdu_buffer[OFFSET_P2];
  pkt->buff_size = G_io_apdu_buffer[OFFSET_LC];
  pkt->buff = &G_io_apdu_buffer[OFFSET_CDATA];
  pkt->is_last = pkt->p1 & P1_LAST_MARKER;
  pkt->is_first = (pkt->p1 & ~P1_LAST_MARKER) == 0;
  if (pkt->buff_size > MAX_APDU_SIZE) THROW(EXC_WRONG_LENGTH_FOR_INS);
}

static int write_signature(uint8_t *const out, uint8_t const *const data, size_t const data_length) {

    key_pair_t key_pair = {0};
    size_t signature_size = 0;

    int error = generate_key_pair(&key_pair,
                                  global.path_with_curve.derivation_type,
                                  &global.path_with_curve.bip32_path);
    if (error) {
        THROW(EXC_WRONG_VALUES);
    }

    error = 0;
    BEGIN_TRY {
        TRY {
            signature_size = sign(out,
                                  MAX_SIGNATURE_SIZE,
                                  global.path_with_curve.derivation_type,
                                  &key_pair,
                                  data,
                                  data_length);
        }
        CATCH_OTHER(e) {
            error = e;
        }
        FINALLY {
            memset(&key_pair, 0, sizeof(key_pair));
        }
    }
    END_TRY;

    if (error) {
        THROW(error);
    }

    return signature_size;
}

static void sign_packet() {
  size_t tx = 0;
  bool send_hash = true; // TODO change this when we handle more APDU instructions

  if (send_hash) {
    memcpy(&G_io_apdu_buffer[tx],
           global.apdu.hash.final_hash,
           sizeof(global.apdu.hash.final_hash));
    tx += sizeof(global.apdu.hash.final_hash);
  }

  tx += write_signature(&G_io_apdu_buffer[tx],
                        global.apdu.hash.final_hash,
                        sizeof(global.apdu.hash.final_hash));
  tx = finalize_successful_send(tx);

  if (global.apdu.sign.step != SIGN_ST_WAIT_USER_INPUT) THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (!(global.apdu.sign.received_last_msg)) THROW(EXC_UNEXPECTED_SIGN_STATE);

  delayed_send(tx);

  global.step = ST_IDLE;
  global.apdu.sign.step = SIGN_ST_IDLE;
}

static void send_reject() {
  if (global.apdu.sign.step != SIGN_ST_WAIT_USER_INPUT) THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (!(global.apdu.sign.received_last_msg)) THROW(EXC_UNEXPECTED_SIGN_STATE);
  clear_data();
  delay_reject();

  global.step = ST_IDLE;
  global.apdu.sign.step = SIGN_ST_IDLE;
}

static void send_continue() {
  if (global.apdu.sign.step != SIGN_ST_WAIT_USER_INPUT) THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (global.apdu.sign.received_last_msg) THROW(EXC_UNEXPECTED_SIGN_STATE);
  delayed_send(finalize_successful_send(0));
  global.apdu.sign.step = SIGN_ST_WAIT_DATA;
}

static void refill() {
  tz_parser_regs *regs = &global.apdu.sign.parser_regs;
  while(!TZ_IS_BLOCKED(tz_operation_parser_step(&global.apdu.sign.parser_state, regs))){};
  PRINTF("[DEBUG] refill(errno: %s) \n", tz_parser_result_name(global.apdu.sign.parser_state.errno));
  switch (global.apdu.sign.parser_state.errno) {
  case TZ_BLO_IM_FULL:
  last_screen:
    tz_ui_stream_push(global.apdu.sign.parser_state.field_name,
                      global.apdu.sign.line_buf);
    tz_parser_regs_flush(&global.apdu.sign.parser_regs,
                         global.apdu.sign.line_buf,
                         TZ_UI_STREAM_CONTENTS_SIZE);
    break;
  case TZ_BLO_FEED_ME:
    send_continue();
    break;
  case TZ_BLO_DONE:
    if (!(global.apdu.sign.received_last_msg) ||
        (global.apdu.sign.parser_regs.ilen != 0)) {
      failwith ("parsing done but some data left");
    }
    if(global.apdu.sign.parser_regs.oofs != 0) goto last_screen;
    tz_ui_stream_close ();
    break;
  default:
    failwith("parsing error");
  }
}

static void stream_cb(tz_ui_cb_type_t type) {
  switch (type) {
  case TZ_UI_STREAM_CB_ACCEPT:
    sign_packet();
    break;
  case TZ_UI_STREAM_CB_REFILL:
    refill();
    break;
  case TZ_UI_STREAM_CB_REJECT:
    send_reject();
    break;
  }
}

static size_t handle_first_apdu(packet_t *pkt) {

  if (global.apdu.sign.step != SIGN_ST_IDLE) THROW(EXC_UNEXPECTED_SIGN_STATE);

  clear_data();
  read_bip32_path(&global.path_with_curve.bip32_path, pkt->buff, pkt->buff_size);
  global.path_with_curve.derivation_type = parse_derivation_type(pkt->p2);

  // init hash
  cx_blake2b_init(&global.apdu.hash.state, SIGN_HASH_SIZE * 8);

  tz_operation_parser_init(&global.apdu.sign.parser_state, TZ_UNKNOWN_SIZE, false);
  tz_parser_regs_refill (&global.apdu.sign.parser_regs, NULL, 0);
  tz_parser_regs_flush(&global.apdu.sign.parser_regs,
                       global.apdu.sign.line_buf,
                       TZ_UI_STREAM_CONTENTS_SIZE);
  tz_ui_stream_init(stream_cb);

  global.apdu.sign.step = SIGN_ST_WAIT_DATA;
  return finalize_successful_send(0);
}

static size_t handle_data_apdu(packet_t *pkt) {
  if (global.apdu.sign.step != SIGN_ST_WAIT_DATA)
    // we received a packet while we did not explicitly asked for one
    THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (global.apdu.sign.parser_regs.ilen > 0)
    // we asked for more input but did not consume what we already had
    THROW(EXC_UNEXPECTED_SIGN_STATE);
  global.apdu.sign.packet_index++; // XXX drop or check

  // do the incremental hashing
  cx_hash((cx_hash_t *)&global.apdu.hash.state,
          pkt->is_last ? CX_LAST : 0,
          pkt->buff, pkt->buff_size,
          global.apdu.hash.final_hash, sizeof(global.apdu.hash.final_hash));
  global.apdu.sign.total_length += pkt->buff_size;

  if (pkt->is_last)
    global.apdu.sign.received_last_msg = true;

  // refill the parser's input
  tz_parser_regs_refill (&global.apdu.sign.parser_regs, pkt->buff, pkt->buff_size);
  if (pkt->is_last) {
    tz_operation_parser_set_size(&global.apdu.sign.parser_state, global.apdu.sign.total_length);
  }

  // resume the parser with the new data
  refill ();

  // loop getting and parsing packets until we have a first screen
  if (tz_ui_stream_current_screen_kind() == TZ_UI_STREAM_DISPLAY_INIT) {
    global.apdu.sign.step = SIGN_ST_WAIT_DATA;
    return finalize_successful_send(0);
  }

  // launch parsing and UI (once we have a first screen)
  global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
  tz_ui_stream ();
}

size_t handle_apdu_sign(__attribute__((unused)) bool return_hash) {
  packet_t pkt;
  init_packet(&pkt);

  if (pkt.is_first) {
    if (global.step != ST_IDLE) THROW (EXC_UNEXPECTED_STATE);
    global.step = ST_SIGN;
    return handle_first_apdu(&pkt);
  } else {
    if (global.step != ST_SIGN) THROW (EXC_UNEXPECTED_STATE);
    return handle_data_apdu(&pkt);
  }
}
