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

#include <memory.h>
#include <string.h>
#include <stdbool.h>

#include <cx.h>
#include <io.h>
#include <os.h>
#include <parser.h>
#include <ux.h>

#include "apdu.h"
#include "apdu_sign.h"
#include "globals.h"
#include "keys.h"
#include "ui_stream.h"

#include "parser/parser_state.h"
#include "parser/operation_parser.h"

#ifdef HAVE_NBGL
#include "nbgl_use_case.h"
#endif

typedef struct {
  command_t cmd;
  bool is_last;
  bool is_first;
} packet_t;

/* Prototypes */

static inline void clear_data(void);
static void init_packet(packet_t *, command_t *);
static tz_err_t sign_packet(void);
static tz_err_t send_reject(void);
static tz_err_t send_continue(void);
static tz_err_t send_cancel(void);
static tz_err_t refill(void);
static tz_err_t stream_cb(tz_ui_cb_type_t);
static void the_cb(tz_ui_cb_type_t);
static tz_err_t handle_first_apdu(packet_t *);
static void   handle_first_apdu_clear(packet_t *);
static void   handle_first_apdu_blind(packet_t *);
static tz_err_t handle_data_apdu(packet_t *);
static tz_err_t handle_data_apdu_clear(packet_t *);
static tz_err_t handle_data_apdu_blind(packet_t *);


/* Macros */

#define P1_FIRST          0x00
#define P1_NEXT           0x01
#define P1_HASH_ONLY_NEXT 0x03  // You only need it once
#define P1_LAST_MARKER    0x80

#define TZ_UI_STREAM_CB_CANCEL 0xf0

#define APDU_SIGN_ASSERT(_cond)  TZ_ASSERT(EXC_UNEXPECTED_SIGN_STATE, (_cond))
#define APDU_SIGN_ASSERT_STEP(x) APDU_SIGN_ASSERT(global.apdu.sign.step == (x))


static inline void clear_data(void) {
    memset(&global.apdu.hash, 0, sizeof(global.apdu.hash));
    memset(&global.apdu.sign, 0, sizeof(global.apdu.sign));
}

static void init_packet(packet_t *pkt, command_t *cmd) {
  pkt->cmd.cla  = cmd->cla;
  pkt->cmd.ins  = cmd->ins;
  pkt->cmd.p1   = cmd->p1;
  pkt->cmd.p2   = cmd->p2;
  pkt->cmd.lc   = cmd->lc;
  pkt->cmd.data = cmd->data;
  pkt->is_last  = cmd->p1 & P1_LAST_MARKER;
  pkt->is_first = (cmd->p1 & ~P1_LAST_MARKER) == 0;
}

static tz_err_t sign_packet(void) {
  size_t siglen = MAX_SIGNATURE_SIZE;
  size_t tx = 0;
  TZ_PREAMBLE(("void"));

  APDU_SIGN_ASSERT_STEP(SIGN_ST_WAIT_USER_INPUT);
  APDU_SIGN_ASSERT(global.apdu.sign.received_last_msg);

  if (global.apdu.sign.return_hash) {
    memcpy(&G_io_apdu_buffer[tx],
           global.apdu.hash.final_hash,
           sizeof(global.apdu.hash.final_hash));
    tx += sizeof(global.apdu.hash.final_hash);
  }

  TZ_CHECK(sign(global.path_with_curve.derivation_type,
                &global.path_with_curve.bip32_path,
                global.apdu.hash.final_hash,
                sizeof(global.apdu.hash.final_hash),
                &G_io_apdu_buffer[tx], &siglen));
  tx += siglen;

  io_send_response_pointer(G_io_apdu_buffer, tx, SW_OK);

  global.step = ST_IDLE;
  global.apdu.sign.step = SIGN_ST_IDLE;

  TZ_POSTAMBLE;
}

static tz_err_t send_reject(void) {
  TZ_PREAMBLE(("void"));

  APDU_SIGN_ASSERT_STEP(SIGN_ST_WAIT_USER_INPUT);
  APDU_SIGN_ASSERT(global.apdu.sign.received_last_msg);

  clear_data();
  io_send_sw(EXC_REJECT);

  global.step = ST_IDLE;
  global.apdu.sign.step = SIGN_ST_IDLE;

  TZ_POSTAMBLE;
}

static tz_err_t send_continue(void) {
  TZ_PREAMBLE(("void"));

  APDU_SIGN_ASSERT(global.apdu.sign.step == SIGN_ST_WAIT_USER_INPUT ||
                   global.apdu.sign.step == SIGN_ST_WAIT_DATA);
  APDU_SIGN_ASSERT(!global.apdu.sign.received_last_msg);

  io_send_sw(SW_OK);
  global.apdu.sign.step = SIGN_ST_WAIT_DATA;

  TZ_POSTAMBLE;
}

static tz_err_t refill() {
  size_t wrote = 0;
  tz_parser_state *st = &global.apdu.sign.u.clear.parser_state;
  TZ_PREAMBLE(("void"));

  while (!TZ_IS_BLOCKED(tz_operation_parser_step(st)))
    ;
  PRINTF("[DEBUG] refill(errno: %s) \n", tz_parser_result_name(st->errno));
  switch (st->errno) {
  case TZ_BLO_IM_FULL:
  last_screen:
    global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
    wrote = tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, st->field_name,
                              global.line_buf, TZ_UI_ICON_NONE);

    tz_parser_flush_up_to(st, global.line_buf,
                          TZ_UI_STREAM_CONTENTS_SIZE, wrote);
    break;
  case TZ_BLO_FEED_ME:
    TZ_CHECK(send_continue());
    break;
  case TZ_BLO_DONE:
    if (!(global.apdu.sign.received_last_msg) ||
        (st->regs.ilen != 0)) {
      failwith("parsing done but some data left");
    }
    if (st->regs.oofs != 0)
      goto last_screen;
    global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
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
    TZ_CHECK(EXC_UNEXPECTED_STATE);
  }

  TZ_POSTAMBLE;
}

static tz_err_t send_cancel(void) {
  tz_parser_state *st = &global.apdu.sign.u.clear.parser_state;
  TZ_PREAMBLE(("void"));

  global.step = ST_IDLE;
  global.apdu.sign.step = SIGN_ST_IDLE;

  switch (st->errno) {
  case TZ_ERR_INVALID_STATE:
    io_send_sw(EXC_UNEXPECTED_STATE);
    break;
  case TZ_ERR_INVALID_TAG:
  case TZ_ERR_INVALID_OP:
  case TZ_ERR_UNSUPPORTED:
  case TZ_ERR_TOO_LARGE:
  case TZ_ERR_TOO_DEEP:
    io_send_sw(EXC_PARSE_ERROR);
    break;
  default:
    TZ_CHECK(EXC_UNEXPECTED_STATE);
  }

  TZ_POSTAMBLE;
}

static tz_err_t stream_cb(tz_ui_cb_type_t type) {
  switch (type) {
  case TZ_UI_STREAM_CB_ACCEPT: return sign_packet();
  case TZ_UI_STREAM_CB_REFILL: return refill();
  case TZ_UI_STREAM_CB_REJECT: return send_reject();
  case TZ_UI_STREAM_CB_CANCEL: return send_cancel();
  default:                     return EXC_UNKNOWN;
  }
}

static tz_err_t handle_first_apdu(packet_t *pkt) {
  TZ_PREAMBLE(("pkt=%p", pkt));

  TZ_ASSERT_NOTNULL(pkt);
  APDU_SIGN_ASSERT_STEP(SIGN_ST_IDLE);

  clear_data();

  TZ_CHECK(read_bip32_path(&global.path_with_curve.bip32_path, pkt->cmd.data,
                           pkt->cmd.lc));
  global.path_with_curve.derivation_type = pkt->cmd.p2;
  TZ_CHECK(check_derivation_type(global.path_with_curve.derivation_type));

  TZ_CHECK(cx_blake2b_init_no_throw(&global.apdu.hash.state,
                                    SIGN_HASH_SIZE * 8));

  tz_ui_stream_init(the_cb);

  switch (global.step) {
  case ST_CLEAR_SIGN: handle_first_apdu_clear(pkt); break;
  case ST_BLIND_SIGN: handle_first_apdu_blind(pkt); break;
  default:            return EXC_UNEXPECTED_STATE;
  }

  global.apdu.sign.step = SIGN_ST_WAIT_DATA;
  io_send_sw(SW_OK);

  TZ_POSTAMBLE;
}

static void handle_first_apdu_clear(__attribute__((unused)) packet_t *pkt) {
  tz_parser_state *st = &global.apdu.sign.u.clear.parser_state;

  tz_operation_parser_init(st, TZ_UNKNOWN_SIZE, false);
  tz_parser_refill(st, NULL, 0);
  tz_parser_flush(st, global.line_buf, TZ_UI_STREAM_CONTENTS_SIZE);
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

static tz_err_t handle_data_apdu(packet_t *pkt) {
  TZ_PREAMBLE(("pkt=%p", pkt));

  APDU_SIGN_ASSERT_STEP(SIGN_ST_WAIT_DATA);
  TZ_ASSERT_NOTNULL(pkt);

  global.apdu.sign.packet_index++; // XXX drop or check

  CX_CHECK(cx_hash_no_throw((cx_hash_t *)&global.apdu.hash.state,
			    pkt->is_last ? CX_LAST : 0,
			    pkt->cmd.data, pkt->cmd.lc,
			    global.apdu.hash.final_hash,
                            sizeof(global.apdu.hash.final_hash)));

  if (pkt->is_last)
    global.apdu.sign.received_last_msg = true;

  switch (global.step) {
  case ST_CLEAR_SIGN: CX_CHECK(handle_data_apdu_clear(pkt)); break;
  case ST_BLIND_SIGN: CX_CHECK(handle_data_apdu_blind(pkt)); break;
  default:            CX_CHECK(EXC_UNEXPECTED_STATE);        break;
  }

  TZ_POSTAMBLE;
}

static tz_err_t handle_data_apdu_clear(packet_t *pkt) {
  tz_parser_state *st = &global.apdu.sign.u.clear.parser_state;
  TZ_PREAMBLE(("pkt=0x%p", pkt));

  TZ_ASSERT_NOTNULL(pkt);
  if (st->regs.ilen > 0)
    // we asked for more input but did not consume what we already had
    TZ_CHECK(EXC_UNEXPECTED_SIGN_STATE);

  global.apdu.sign.u.clear.total_length += pkt->cmd.lc;

  tz_parser_refill(st, pkt->cmd.data, pkt->cmd.lc);
  if (pkt->is_last)
    tz_operation_parser_set_size(st, global.apdu.sign.u.clear.total_length);
  TZ_CHECK(refill());
  tz_ui_stream();

  TZ_POSTAMBLE;
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
static tz_err_t handle_data_apdu_blind(packet_t *pkt) {
  TZ_PREAMBLE(("pkt=0x%p", pkt));

  TZ_ASSERT_NOTNULL(pkt);
  if (!global.apdu.sign.u.blind.tag)
    global.apdu.sign.u.blind.tag = pkt->cmd.data[0];
  if (!pkt->is_last)
    return io_send_sw(SW_OK);

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

  /* XXXrcd: the logic here need analysis. */
  TZ_POSTAMBLE;
}
#undef FINAL_HASH

/* XXXrcd: all of the THROW()s should be below this line */

static void the_cb(tz_ui_cb_type_t type) {
  CX_THROW(stream_cb(type));
}

tz_err_t handle_apdu_sign(command_t *cmd) {
  bool return_hash = cmd->ins == INS_SIGN_WITH_HASH;
  packet_t pkt;
  TZ_PREAMBLE(("cmd=0x%p", cmd));

  init_packet(&pkt, cmd);
  TZ_ASSERT(EXC_WRONG_LENGTH_FOR_INS, pkt.cmd.lc <= MAX_APDU_SIZE);

PRINTF("pkt.is_first: %x\n", pkt.is_first);
  if (pkt.is_first) {
    TZ_ASSERT(EXC_UNEXPECTED_STATE, global.step == ST_IDLE);

    memset(&global.apdu, 0, sizeof(global.apdu));

    #ifdef HAVE_BAGL
    switch (tz_ui_stream_get_type()) {
    #elif HAVE_NBGL
    switch (global.home_screen) {
    #endif
    case SCREEN_CLEAR_SIGN: global.step = ST_CLEAR_SIGN; break;
    case SCREEN_BLIND_SIGN: global.step = ST_BLIND_SIGN; break;
    default:
      /* XXXrcd: WRONG */
      CX_CHECK(EXC_UNEXPECTED_STATE);
    }

    TZ_CHECK(handle_first_apdu(&pkt));
    global.apdu.sign.return_hash = return_hash;
    goto end;
  }

  TZ_ASSERT(EXC_UNEXPECTED_STATE,
            global.step == ST_BLIND_SIGN || global.step == ST_CLEAR_SIGN);
  TZ_ASSERT(EXC_INVALID_INS, return_hash == global.apdu.sign.return_hash);

  TZ_CHECK(handle_data_apdu(&pkt));

  TZ_POSTAMBLE;
}
