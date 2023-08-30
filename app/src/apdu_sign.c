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

#ifdef HAVE_NBGL
#include "nbgl_use_case.h"
#endif

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
static void sign_packet(void);
static void send_reject(void);
static void send_continue(void);
static void refill(void);
static void stream_cb(tz_ui_cb_type_t);
static size_t handle_first_apdu(packet_t *, bool);
static void   handle_first_apdu_clear(packet_t *);
static void   handle_first_apdu_blind(packet_t *);
static size_t handle_data_apdu(packet_t *);
static size_t handle_data_apdu_clear(packet_t *);
static size_t handle_data_apdu_blind(packet_t *);


#define P1_FIRST          0x00
#define P1_NEXT           0x01
#define P1_HASH_ONLY_NEXT 0x03  // You only need it once
#define P1_LAST_MARKER    0x80

#define TZ_UI_STREAM_CB_CANCEL 0xf0


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

static void sign_packet() {
  size_t siglen = MAX_SIGNATURE_SIZE;
  size_t tx = 0;
  cx_err_t err;

  FUNC_ENTER(("void"));
  if (global.apdu.sign.return_hash) {
    memcpy(&G_io_apdu_buffer[tx],
           global.apdu.hash.final_hash,
           sizeof(global.apdu.hash.final_hash));
    tx += sizeof(global.apdu.hash.final_hash);
  }

  err = sign(global.path_with_curve.derivation_type,
             &global.path_with_curve.bip32_path,
             global.apdu.hash.final_hash, sizeof(global.apdu.hash.final_hash),
             &G_io_apdu_buffer[tx], &siglen);
  if (err)
    THROW(err);

  tx += siglen;
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
  if (global.apdu.sign.step != SIGN_ST_WAIT_USER_INPUT &&
      global.apdu.sign.step != SIGN_ST_WAIT_DATA)
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
    global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
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
  case TZ_ERR_INVALID_STATE:
    tz_ui_stream_push(TZ_UI_STREAM_CB_CANCEL,
                      "Unknown error",
                      "",
                      TZ_UI_ICON_CROSS);
    tz_ui_stream_close();
    break;
  case TZ_ERR_INVALID_TAG:
  case TZ_ERR_INVALID_OP:
  case TZ_ERR_UNSUPPORTED:
  case TZ_ERR_TOO_LARGE:
  case TZ_ERR_TOO_DEEP:
    tz_ui_stream_push(TZ_UI_STREAM_CB_CANCEL,
                      "Parsing error",
                      tz_parser_result_name(st->errno),
                      TZ_UI_ICON_CROSS);
    tz_ui_stream_close();
    break;
  default:
    THROW(EXC_UNEXPECTED_STATE);
  }
  FUNC_LEAVE();
}

static void send_cancel() {
  tz_parser_state *st = &global.apdu.sign.u.clear.parser_state;

  FUNC_ENTER(("void"));
  switch (st->errno) {
  case TZ_ERR_INVALID_STATE:
    delay_exc(EXC_UNEXPECTED_STATE);
    break;
  case TZ_ERR_INVALID_TAG:
  case TZ_ERR_INVALID_OP:
  case TZ_ERR_UNSUPPORTED:
  case TZ_ERR_TOO_LARGE:
  case TZ_ERR_TOO_DEEP:
    delay_exc(EXC_PARSE_ERROR);
    break;
  default:
    THROW(EXC_UNEXPECTED_STATE);
  }
  clear_data();
  global.step = ST_IDLE;
  global.apdu.sign.step = SIGN_ST_IDLE;
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
  case TZ_UI_STREAM_CB_CANCEL:
    send_cancel();
    break;
  }
}

static size_t handle_first_apdu(packet_t *pkt, bool return_hash) {

  FUNC_ENTER(("pkt=%p", pkt));
  if (global.apdu.sign.step != SIGN_ST_IDLE) THROW(EXC_UNEXPECTED_SIGN_STATE);

  clear_data();
  global.apdu.sign.return_hash = return_hash;

  CX_THROW(read_bip32_path(&global.path_with_curve.bip32_path, pkt->buff,
                           pkt->buff_size));
  global.path_with_curve.derivation_type = pkt->p2;
  CX_THROW(check_derivation_type(global.path_with_curve.derivation_type));

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
#ifdef HAVE_NBGL
  nbgl_useCaseSpinner("Loading operation");
#endif

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

  tz_parser_refill(st, pkt->buff, pkt->buff_size);
  if (pkt->is_last)
    tz_operation_parser_set_size(st, global.apdu.sign.u.clear.total_length);
  refill();
  tz_ui_stream();
  FUNC_LEAVE();
}

#ifdef HAVE_NBGL
static nbgl_layoutTagValueList_t useCaseTagValueList;
static nbgl_pageInfoLongPress_t infoLongPress;

void reject_blindsign_cb(void) {
  FUNC_ENTER(("void"));

  stream_cb(TZ_UI_STREAM_CB_REJECT);
  ui_home_init();

  FUNC_LEAVE();
}

void accept_blindsign_cb(void) {
  FUNC_ENTER(("void"));

  stream_cb(TZ_UI_STREAM_CB_ACCEPT);
  ui_home_init();

  FUNC_LEAVE();
}

static void reviewChoice(bool confirm) {
  FUNC_ENTER(("confirm=%d", confirm));

  if (confirm) {
    nbgl_useCaseStatus("SIGNING\nSUCCESSFUL", true, accept_blindsign_cb);
  } else {
    nbgl_useCaseStatus("Rejected", false, reject_blindsign_cb);
  }

  FUNC_LEAVE();
}

static const char* transaction_type;
static char hash[TZ_BASE58_BUFFER_SIZE(sizeof(global.apdu.hash.final_hash))];
static nbgl_layoutTagValue_t pair;
static nbgl_layoutTagValue_t* getTagValuePair(uint8_t pairIndex) {
  switch (pairIndex) {
    case 0:
        pair.item = "Type";
        pair.value = transaction_type;
        break;
    case 1:
      pair.item = "Hash";
      pair.value = hash;
      break;
    default:
      return NULL;
  }
  return &pair;
}

void continue_blindsign_cb(void) {
  FUNC_ENTER(("void"));

  useCaseTagValueList.pairs = NULL;
  useCaseTagValueList.callback = getTagValuePair;
  useCaseTagValueList.startIndex = 0;
  useCaseTagValueList.nbPairs = 2;
  useCaseTagValueList.smallCaseForValue = false;
  useCaseTagValueList.wrapping = false;
  infoLongPress.icon = &C_tezos;
  infoLongPress.text = "Approve";
  infoLongPress.longPressText = "Sign";
  nbgl_useCaseStaticReview(&useCaseTagValueList, &infoLongPress, "Reject", reviewChoice);

  FUNC_LEAVE();
}

#endif

#define FINAL_HASH global.apdu.hash.final_hash
static size_t handle_data_apdu_blind(packet_t *pkt) {
  if (!global.apdu.sign.u.blind.tag)
    global.apdu.sign.u.blind.tag = pkt->buff[0];
  if (pkt->is_last) {
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

#ifdef HAVE_BAGL
    tz_ui_stream_push_all(TZ_UI_STREAM_CB_NOCB, "Sign Hash", type,
                          TZ_UI_ICON_NONE);

    tz_ui_stream_push_all(TZ_UI_STREAM_CB_NOCB, "Sign Hash", obuf,
                          TZ_UI_ICON_NONE);

    tz_ui_stream_push_accept_reject();
#endif

#ifdef HAVE_NBGL
    char request[80];
    snprintf(request, sizeof(request), "Review request to blind\nsign %s", type);
    global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;

    transaction_type = type;
    strcpy(hash, obuf);

    nbgl_useCaseReviewStart(&C_tezos,
                            request,
                            NULL,
                            "Reject request",
                            continue_blindsign_cb,
                            reject_blindsign_cb
                            );
#endif

    tz_ui_stream_close();
    tz_ui_stream();
  } else {
    return finalize_successful_send(0);
  }

  // return 0;
}
#undef FINAL_HASH

size_t handle_apdu_sign(bool return_hash) {
  packet_t pkt;
  size_t ret;

  FUNC_ENTER(("return_hash=%s", return_hash ? "true" : "false"));

  init_packet(&pkt);

  if (pkt.is_first) {
    if (global.step != ST_IDLE)
      THROW(EXC_UNEXPECTED_STATE);

    #ifdef HAVE_BAGL
    switch (tz_ui_stream_get_type()) {
    #elif HAVE_NBGL
    switch (global.home_screen) {
    #endif
    case SCREEN_CLEAR_SIGN: global.step = ST_CLEAR_SIGN; break;
    case SCREEN_BLIND_SIGN: global.step = ST_BLIND_SIGN; break;
    default:
      /* XXXrcd: WRONG */
      THROW(EXC_UNEXPECTED_STATE);
    }

    ret = handle_first_apdu(&pkt, return_hash);
  } else {
    if (global.step != ST_BLIND_SIGN && global.step != ST_CLEAR_SIGN)
      THROW(EXC_UNEXPECTED_STATE);
    if (return_hash != global.apdu.sign.return_hash)
      THROW(EXC_INVALID_INS);

    ret = handle_data_apdu(&pkt);
  }

  FUNC_LEAVE();
  return ret;
}
