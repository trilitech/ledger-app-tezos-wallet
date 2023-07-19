/* Tezos Ledger application - Clear signing command handler

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 TriliTech <contact@trili.tech>
   Copyright 2023 Functori <contact@functori.com>

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

/* Prototypes */

static inline void clear_data(void);
static void init_packet(packet_t *);
static int write_signature(uint8_t *, const uint8_t *, size_t);
static void sign_packet(void);
static void send_reject(void);
static void send_continue(void);
static void refill(void);
static void stream_cb(tz_ui_cb_type_t);
static size_t handle_first_apdu(packet_t *);
static void   handle_first_apdu_clear(packet_t *);
static void   handle_first_apdu_blind(packet_t *);
static size_t handle_data_apdu(packet_t *);
static size_t handle_data_apdu_clear(packet_t *);
static size_t handle_data_apdu_blind(packet_t *);


#define P1_FIRST          0x00
#define P1_NEXT           0x01
#define P1_HASH_ONLY_NEXT 0x03  // You only need it once
#define P1_LAST_MARKER    0x80


static inline void clear_data(void) {
    memset(&global.apdu.hash, 0, sizeof(global.apdu.hash));
    memset(&global.apdu.sign, 0, sizeof(global.apdu.sign));
}

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

static int write_signature(uint8_t *const out, uint8_t const *const data,
                           size_t const data_length) {

    FUNC_ENTER(("out=%p, data=%p, data_length=%u", out, data, data_length));

    cx_ecfp_private_key_t priv;
    size_t signature_size = 0;
    int error;


    error = crypto_derive_private_key(&priv,
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
                                  &priv,
                                  data,
                                  data_length);
        }
        CATCH_OTHER(e) {
            error = e;
        }
        FINALLY {
            memset(&priv, 0, sizeof(priv));
        }
    }
    END_TRY;

    if (error) {
        THROW(error);
    }

    FUNC_LEAVE();
    return signature_size;
}

static void sign_packet() {
  size_t tx = 0;
  bool send_hash = true; // TODO change this when we handle more
                         // APDU instructions

  FUNC_ENTER(("void"));
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

  if (global.apdu.sign.step != SIGN_ST_WAIT_USER_INPUT)
    THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (!(global.apdu.sign.received_last_msg))
    THROW(EXC_UNEXPECTED_SIGN_STATE);

  delayed_send(tx);

  global.step = ST_IDLE;
  global.apdu.sign.step = SIGN_ST_IDLE;
  FUNC_LEAVE();
}

static void send_reject() {
  FUNC_ENTER(("void"));
  if (global.apdu.sign.step != SIGN_ST_WAIT_USER_INPUT)
    THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (!(global.apdu.sign.received_last_msg))
    THROW(EXC_UNEXPECTED_SIGN_STATE);
  clear_data();
  delay_reject();

  global.step = ST_IDLE;
  global.apdu.sign.step = SIGN_ST_IDLE;
  FUNC_LEAVE();
}

static void send_continue() {
  FUNC_ENTER(("void"));
  if (global.apdu.sign.step != SIGN_ST_WAIT_USER_INPUT)
    THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (global.apdu.sign.received_last_msg)
    THROW(EXC_UNEXPECTED_SIGN_STATE);
  delayed_send(finalize_successful_send(0));
  global.apdu.sign.step = SIGN_ST_WAIT_DATA;
  FUNC_LEAVE();
}

static void refill() {
  size_t wrote = 0;
  tz_parser_state *st = &global.apdu.sign.u.clear.parser_state;

  FUNC_ENTER(("void"));
  while (!TZ_IS_BLOCKED(tz_operation_parser_step(st)))
    ;
  PRINTF("[DEBUG] refill(errno: %s) \n", tz_parser_result_name(st->errno));
  switch (st->errno) {
  case TZ_BLO_IM_FULL:
  last_screen:
    wrote = tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, st->field_name,
                              global.apdu.sign.line_buf, TZ_UI_ICON_NONE);

    tz_parser_flush_up_to(st, global.apdu.sign.line_buf,
                          TZ_UI_STREAM_CONTENTS_SIZE, wrote);
    break;
  case TZ_BLO_FEED_ME:
    send_continue();
    break;
  case TZ_BLO_DONE:
    if (!(global.apdu.sign.received_last_msg) ||
        (st->regs.ilen != 0)) {
      failwith("parsing done but some data left");
    }
    if (st->regs.oofs != 0)
      goto last_screen;
    tz_ui_stream_push_accept_reject();
    tz_ui_stream_close();
    break;
  default:
    failwith("parsing error");
  }
  FUNC_LEAVE();
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

  FUNC_ENTER(("pkt=%p", pkt));
  if (global.apdu.sign.step != SIGN_ST_IDLE) THROW(EXC_UNEXPECTED_SIGN_STATE);

  clear_data();
  read_bip32_path(&global.path_with_curve.bip32_path, pkt->buff,
                  pkt->buff_size);
  global.path_with_curve.derivation_type = parse_derivation_type(pkt->p2);

  // init hash
  CX_THROW(cx_blake2b_init_no_throw(&global.apdu.hash.state,
                                    SIGN_HASH_SIZE * 8));

  tz_ui_stream_init(stream_cb);

  switch (global.step) {
  case ST_CLEAR_SIGN: handle_first_apdu_clear(pkt); break;
  case ST_BLIND_SIGN: handle_first_apdu_blind(pkt); break;
  default:
    THROW(EXC_UNEXPECTED_STATE);
  }

  global.apdu.sign.step = SIGN_ST_WAIT_DATA;
  FUNC_LEAVE();
  return finalize_successful_send(0);
}

static void handle_first_apdu_clear(__attribute__((unused)) packet_t *pkt) {
  tz_parser_state *st = &global.apdu.sign.u.clear.parser_state;

  tz_operation_parser_init(st, TZ_UNKNOWN_SIZE, false);
  tz_parser_refill(st, NULL, 0);
  tz_parser_flush(st, global.apdu.sign.line_buf, TZ_UI_STREAM_CONTENTS_SIZE);
}

static void handle_first_apdu_blind(__attribute__((unused)) packet_t *pkt) {

  /*
   * We set the tag to zero here which indicates that it is unset.
   * The first data packet will set it to the first byte.
   */

  global.apdu.sign.u.blind.tag = 0;
}

static size_t handle_data_apdu(packet_t *pkt) {
  FUNC_ENTER(("pkt=%p", pkt));
  if (global.apdu.sign.step != SIGN_ST_WAIT_DATA)
    // we received a packet while we did not explicitly asked for one
    THROW(EXC_UNEXPECTED_SIGN_STATE);
  global.apdu.sign.packet_index++; // XXX drop or check

  // do the incremental hashing
  CX_THROW(cx_hash_no_throw((cx_hash_t *)&global.apdu.hash.state,
			    pkt->is_last ? CX_LAST : 0,
			    pkt->buff, pkt->buff_size,
			    global.apdu.hash.final_hash,
                            sizeof(global.apdu.hash.final_hash)));

  if (pkt->is_last)
    global.apdu.sign.received_last_msg = true;

  switch (global.step) {
  case ST_CLEAR_SIGN: return handle_data_apdu_clear(pkt);
  case ST_BLIND_SIGN: return handle_data_apdu_blind(pkt);
  default:
    THROW(EXC_UNEXPECTED_STATE);
  }
}

static size_t handle_data_apdu_clear(packet_t *pkt) {
  tz_parser_state *st = &global.apdu.sign.u.clear.parser_state;

  if (st->regs.ilen > 0)
    // we asked for more input but did not consume what we already had
    THROW(EXC_UNEXPECTED_SIGN_STATE);

  global.apdu.sign.u.clear.total_length += pkt->buff_size;

  // refill the parser's input
  tz_parser_refill(st, pkt->buff, pkt->buff_size);
  if (pkt->is_last)
    tz_operation_parser_set_size(st, global.apdu.sign.u.clear.total_length);

  // resume the parser with the new data
  refill();

  // loop getting and parsing packets until we have a first screen
  if (tz_ui_stream_current_screen_kind() == TZ_UI_STREAM_DISPLAY_INIT) {
    global.apdu.sign.step = SIGN_ST_WAIT_DATA;
    return finalize_successful_send(0);
  }

  // launch parsing and UI (once we have a first screen)
  global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
  tz_ui_stream();
  FUNC_LEAVE();
}

#define FINAL_HASH global.apdu.hash.final_hash
static size_t handle_data_apdu_blind(packet_t *pkt) {
  if (!global.apdu.sign.u.blind.tag)
    global.apdu.sign.u.blind.tag = pkt->buff[0];
  if (pkt->is_last) {
    size_t i = 0;
    char obuf[TZ_BASE58_BUFFER_SIZE(sizeof(FINAL_HASH))];
    const char *type = "unknown type";

    global.apdu.sign.received_last_msg = true;
    global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;

    tz_format_base58(FINAL_HASH, sizeof(FINAL_HASH), obuf);

    switch(global.apdu.sign.u.blind.tag) {
    case 0x01: case 0x11: type = "Block\nproposal";         break;
    case 0x03:            type = "Manager\noperation";      break;
    case 0x02:
    case 0x12: case 0x13: type = "Consensus\noperation";    break;
    case 0x05:            type = "Micheline\nexpression";   break;
    default:                                                break;
    }

    tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Sign Hash", type,
                      TZ_UI_ICON_NONE);

    size_t obuflen = strlen(obuf);
    do {
      i += tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Sign Hash", obuf + i,
                             TZ_UI_ICON_NONE);
    } while (i < obuflen);

    tz_ui_stream_push_accept_reject();
    tz_ui_stream_close();
    tz_ui_stream();
  } else {
    return finalize_successful_send(0);
  }

  // return 0;
}
#undef FINAL_HASH

size_t handle_apdu_sign(__attribute__((unused)) bool return_hash) {
  packet_t pkt;
  size_t ret;

  FUNC_ENTER(("return_hash=%s", return_hash ? "true" : "false"));

  init_packet(&pkt);

  if (pkt.is_first) {
    if (global.step != ST_IDLE)
      THROW(EXC_UNEXPECTED_STATE);

    switch (tz_ui_stream_get_type()) {
    case SCREEN_CLEAR_SIGN: global.step = ST_CLEAR_SIGN; break;
    case SCREEN_BLIND_SIGN: global.step = ST_BLIND_SIGN; break;
    default:
      /* XXXrcd: WRONG */
      THROW(EXC_UNEXPECTED_STATE);
    }

    ret = handle_first_apdu(&pkt);
  } else {
    if (global.step != ST_BLIND_SIGN && global.step != ST_CLEAR_SIGN)
      THROW(EXC_UNEXPECTED_STATE);

    ret = handle_data_apdu(&pkt);
  }

  FUNC_LEAVE();
  return ret;
}
