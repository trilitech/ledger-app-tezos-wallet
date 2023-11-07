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
#include "handle_swap.h"
#include "keys.h"
#include "ui_stream.h"

#include "parser/parser_state.h"
#include "parser/operation_parser.h"

#ifdef HAVE_NBGL
#include "nbgl_use_case.h"
#endif

/* Prototypes */

static void sign_packet(void);
static void send_reject(void);
static void send_continue(void);
static void send_cancel(void);
static void refill(void);
static void stream_cb(tz_ui_cb_type_t);
static void handle_first_apdu(command_t *);
static void handle_first_apdu_clear(command_t *);
static void handle_first_apdu_blind(command_t *);
static void handle_data_apdu(command_t *);
static void handle_data_apdu_clear(command_t *);
static void handle_data_apdu_blind(command_t *);

/* Macros */

#define P1_FIRST          0x00
#define P1_NEXT           0x01
#define P1_HASH_ONLY_NEXT 0x03  // You only need it once
#define P1_LAST_MARKER    0x80

#define PKT_IS_LAST(_cmd)  ((_cmd)->p1 & P1_LAST_MARKER)
#define PKT_IS_FIRST(_cmd) (((_cmd)->p1 & ~P1_LAST_MARKER) == 0)

#define APDU_SIGN_ASSERT(_cond) TZ_ASSERT(EXC_UNEXPECTED_SIGN_STATE, (_cond))
#define APDU_SIGN_ASSERT_STEP(x) \
    APDU_SIGN_ASSERT(global.keys.apdu.sign.step == (x))

#ifdef HAVE_BAGL
void
tz_ui_stream_push_accept_reject(void)
{
    FUNC_ENTER(("void"));
#ifdef TARGET_NANOS
    tz_ui_stream_push(TZ_UI_STREAM_CB_ACCEPT, "Accept and send", "",
                      TZ_UI_LAYOUT_BP, TZ_UI_ICON_TICK);
#else
    tz_ui_stream_push(TZ_UI_STREAM_CB_ACCEPT, "Accept", "and send",
                      TZ_UI_LAYOUT_BP, TZ_UI_ICON_TICK);
#endif
    tz_ui_stream_push(TZ_UI_STREAM_CB_REJECT, "Reject", "", TZ_UI_LAYOUT_BP,
                      TZ_UI_ICON_CROSS);
    FUNC_LEAVE();
}
#endif

static void
sign_packet(void)
{
    buffer_t bufs[2] = {0};
    uint8_t  sig[MAX_SIGNATURE_SIZE];
    TZ_PREAMBLE(("void"));

    APDU_SIGN_ASSERT_STEP(SIGN_ST_WAIT_USER_INPUT);
    APDU_SIGN_ASSERT(global.keys.apdu.sign.received_last_msg);

    TZ_CHECK(swap_check_validity());

    bufs[0].ptr  = global.keys.apdu.hash.final_hash;
    bufs[0].size = sizeof(global.keys.apdu.hash.final_hash);
    bufs[1].ptr  = sig;
    bufs[1].size = sizeof(sig);
    TZ_CHECK(sign(global.path_with_curve.derivation_type,
                  &global.path_with_curve.bip32_path, bufs[0].ptr,
                  bufs[0].size, sig, &bufs[1].size));

    /* If we aren't returning the hash, zero its buffer... */
    if (!global.keys.apdu.sign.return_hash)
        bufs[0].size = 0;

    io_send_response_buffers(bufs, 2, SW_OK);
    global.step = ST_IDLE;

    TZ_POSTAMBLE;
}

static void
send_reject(void)
{
    TZ_PREAMBLE(("void"));

    APDU_SIGN_ASSERT_STEP(SIGN_ST_WAIT_USER_INPUT);
    APDU_SIGN_ASSERT(global.keys.apdu.sign.received_last_msg);
    TZ_FAIL(EXC_REJECT);
    TZ_POSTAMBLE;
}

static void
send_continue(void)
{
    TZ_PREAMBLE(("void"));

    APDU_SIGN_ASSERT(global.keys.apdu.sign.step == SIGN_ST_WAIT_USER_INPUT
                     || global.keys.apdu.sign.step == SIGN_ST_WAIT_DATA);
    APDU_SIGN_ASSERT(!global.keys.apdu.sign.received_last_msg);

    io_send_sw(SW_OK);
    global.keys.apdu.sign.step = SIGN_ST_WAIT_DATA;

    TZ_POSTAMBLE;
}

static void
refill_blo_im_full(void)
{
    tz_parser_state *st    = &global.keys.apdu.sign.u.clear.parser_state;
    size_t           wrote = 0;
    TZ_PREAMBLE(("void"));

    if (!global.keys.apdu.sign.u.clear.skip_to_sign) {
        global.keys.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
        wrote = tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, st->field_name,
                                  global.line_buf, TZ_UI_LAYOUT_BNP,
                                  TZ_UI_ICON_NONE);
    } else {
        global.keys.apdu.sign.step = SIGN_ST_WAIT_DATA;
        wrote                      = TZ_UI_STREAM_CONTENTS_SIZE;
    }
    tz_parser_flush_up_to(st, global.line_buf, TZ_UI_STREAM_CONTENTS_SIZE,
                          wrote);
    TZ_POSTAMBLE;
}

static void
refill_blo_done(void)
{
    tz_parser_state *st = &global.keys.apdu.sign.u.clear.parser_state;
    TZ_PREAMBLE(("void"));

    TZ_ASSERT(EXC_UNEXPECTED_STATE,
              global.keys.apdu.sign.received_last_msg && st->regs.ilen == 0);
    if (st->regs.oofs != 0) {
        refill_blo_im_full();
        TZ_SUCCEED();
    }
    global.keys.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
#ifdef HAVE_BAGL
    tz_ui_stream_push_accept_reject();
#endif
    tz_ui_stream_close();

    TZ_POSTAMBLE;
}

static void
refill_error(void)
{
    tz_parser_state *st = &global.keys.apdu.sign.u.clear.parser_state;
    TZ_PREAMBLE(("void"));

    tz_ui_stream_push(TZ_UI_STREAM_CB_CANCEL, "Parsing error",
                      tz_parser_result_name(st->errno), TZ_UI_LAYOUT_BNP,
                      TZ_UI_ICON_CROSS);

    tz_ui_stream_close();
    TZ_POSTAMBLE;
}

static void
refill(void)
{
    tz_parser_state *st  = &global.keys.apdu.sign.u.clear.parser_state;
    bool             s2s = global.keys.apdu.sign.u.clear.skip_to_sign;
    TZ_PREAMBLE(("void"));

    do {
        while (!TZ_IS_BLOCKED(tz_operation_parser_step(st)))
            ;
        PRINTF("[DEBUG] refill(errno: %s)\n",
               tz_parser_result_name(st->errno));
        // clang-format off
        switch (st->errno) {
        case TZ_BLO_IM_FULL: TZ_CHECK(refill_blo_im_full()); break;
        case TZ_BLO_FEED_ME: TZ_CHECK(send_continue());      break;
        case TZ_BLO_DONE:    TZ_CHECK(refill_blo_done());    break;
        default:             TZ_CHECK(refill_error());       break;
        }
        // clang-format on
    } while (s2s
             && (st->errno == TZ_BLO_IM_FULL
                 || (st->errno == TZ_BLO_DONE && st->regs.oofs > 0)));

    TZ_POSTAMBLE;
}

static void
send_cancel(void)
{
    tz_parser_state *st = &global.keys.apdu.sign.u.clear.parser_state;
    TZ_PREAMBLE(("void"));

    global.keys.apdu.sign.step = SIGN_ST_IDLE;

    switch (st->errno) {
    case TZ_ERR_INVALID_STATE:
        TZ_FAIL(EXC_UNEXPECTED_STATE);
        break;
    case TZ_ERR_INVALID_TAG:
    case TZ_ERR_INVALID_OP:
    case TZ_ERR_INVALID_DATA:
    case TZ_ERR_UNSUPPORTED:
    case TZ_ERR_TOO_LARGE:
    case TZ_ERR_TOO_DEEP:
        TZ_FAIL(EXC_PARSE_ERROR);
        break;
    default:
        TZ_FAIL(EXC_UNEXPECTED_STATE);
    }

    TZ_POSTAMBLE;
}

static void
stream_cb(tz_ui_cb_type_t cb_type)
{
    TZ_PREAMBLE(("cb_type=%u", cb_type));

    // clang-format off
    switch (cb_type) {
    case TZ_UI_STREAM_CB_ACCEPT: return sign_packet();
    case TZ_UI_STREAM_CB_REFILL: return refill();
    case TZ_UI_STREAM_CB_REJECT: return send_reject();
    case TZ_UI_STREAM_CB_CANCEL: return send_cancel();
    default:                     TZ_FAIL(EXC_UNKNOWN);
    }
    // clang-format on

    TZ_POSTAMBLE;
}

#define FINAL_HASH global.keys.apdu.hash.final_hash
#ifdef HAVE_BAGL
static void
bs_push_next()
{
    char              obuf[TZ_BASE58_BUFFER_SIZE(sizeof(FINAL_HASH))];
    blindsign_step_t *step = &global.keys.apdu.sign.u.blind.step;

    TZ_PREAMBLE(("void"));

    switch (*step) {
    case BLINDSIGN_ST_OPERATION:
        *step = BLINDSIGN_ST_HASH;

        if (tz_format_base58(FINAL_HASH, sizeof(FINAL_HASH), obuf,
                             sizeof(obuf)))
            TZ_FAIL(EXC_UNKNOWN);

        tz_ui_stream_push_all(TZ_UI_STREAM_CB_NOCB, "Sign Hash", obuf,
                              TZ_UI_LAYOUT_BNP, TZ_UI_ICON_NONE);
        break;
    case BLINDSIGN_ST_HASH:
        *step = BLINDSIGN_ST_ACCEPT_REJECT;

        tz_ui_stream_push_accept_reject();
        tz_ui_stream_close();
        break;
    default:
        PRINTF("Unexpected blindsign state: %d\n", (int)*step);
        TZ_FAIL(EXC_UNEXPECTED_STATE);
    };

    TZ_POSTAMBLE;
}

static void
bs_stream_cb(tz_ui_cb_type_t cb_type)
{
    TZ_PREAMBLE(("cb_type=%u", cb_type));

    // clang-format off
    switch (cb_type) {
    case TZ_UI_STREAM_CB_ACCEPT: return sign_packet();
    case TZ_UI_STREAM_CB_REFILL: return bs_push_next();
    case TZ_UI_STREAM_CB_REJECT: return send_reject();
    case TZ_UI_STREAM_CB_CANCEL: return send_cancel();
    default:                     TZ_FAIL(EXC_UNKNOWN);
    }
    // clang-format on

    TZ_POSTAMBLE;
}
#endif  // HAVE_BAGL

static void
handle_first_apdu(command_t *cmd)
{
    TZ_PREAMBLE(("cmd=%p", cmd));

    TZ_ASSERT_NOTNULL(cmd);
    APDU_SIGN_ASSERT_STEP(SIGN_ST_IDLE);

    TZ_CHECK(read_bip32_path(&global.path_with_curve.bip32_path, cmd->data,
                             cmd->lc));
    global.path_with_curve.derivation_type = cmd->p2;
    TZ_ASSERT(EXC_WRONG_PARAM,
              check_derivation_type(global.path_with_curve.derivation_type));
    TZ_CHECK(cx_blake2b_init_no_throw(&global.keys.apdu.hash.state,
                                      SIGN_HASH_SIZE * 8));
    // clang-format off
    switch (global.step) {
    case ST_CLEAR_SIGN: TZ_CHECK(handle_first_apdu_clear(cmd)); break;
    case ST_BLIND_SIGN: TZ_CHECK(handle_first_apdu_blind(cmd)); break;
    default:            TZ_FAIL(EXC_UNEXPECTED_STATE);
    }
    // clang-format on

    io_send_sw(SW_OK);
    global.keys.apdu.sign.step = SIGN_ST_WAIT_DATA;

    TZ_POSTAMBLE;
}

static void
handle_first_apdu_clear(__attribute__((unused)) command_t *cmd)
{
    tz_parser_state *st = &global.keys.apdu.sign.u.clear.parser_state;

    tz_ui_stream_init(stream_cb);

    tz_operation_parser_init(st, TZ_UNKNOWN_SIZE, false);
    tz_parser_refill(st, NULL, 0);
    tz_parser_flush(st, global.line_buf, TZ_UI_STREAM_CONTENTS_SIZE);
}

static void
handle_first_apdu_blind(__attribute__((unused)) command_t *cmd)
{
#ifdef HAVE_BAGL
    tz_ui_stream_init(bs_stream_cb);
#elif HAVE_NBGL
    nbgl_useCaseSpinner("Loading operation");
#endif

    /*
     * We set the tag to zero here which indicates that it is unset.
     * The first data packet will set it to the first byte.
     */
    global.keys.apdu.sign.u.blind.tag  = 0;
    global.keys.apdu.sign.u.blind.step = BLINDSIGN_ST_OPERATION;
}

static void
handle_data_apdu(command_t *cmd)
{
    TZ_PREAMBLE(("cmd=%p", cmd));

    APDU_SIGN_ASSERT_STEP(SIGN_ST_WAIT_DATA);
    TZ_ASSERT_NOTNULL(cmd);

    global.keys.apdu.sign.packet_index++;  // XXX drop or check

    CX_CHECK(cx_hash_no_throw((cx_hash_t *)&global.keys.apdu.hash.state,
                              PKT_IS_LAST(cmd) ? CX_LAST : 0, cmd->data,
                              cmd->lc, global.keys.apdu.hash.final_hash,
                              sizeof(global.keys.apdu.hash.final_hash)));

    if (PKT_IS_LAST(cmd))
        global.keys.apdu.sign.received_last_msg = true;

    // clang-format off
    switch (global.step) {
    case ST_CLEAR_SIGN: TZ_CHECK(handle_data_apdu_clear(cmd)); break;
    case ST_BLIND_SIGN: TZ_CHECK(handle_data_apdu_blind(cmd)); break;
    default:            TZ_FAIL(EXC_UNEXPECTED_STATE);         break;
    }
    // clang-format on

    TZ_POSTAMBLE;
}

static void
handle_data_apdu_clear(command_t *cmd)
{
    tz_parser_state *st = &global.keys.apdu.sign.u.clear.parser_state;
    TZ_PREAMBLE(("cmd=0x%p", cmd));

    TZ_ASSERT_NOTNULL(cmd);
    if (st->regs.ilen > 0)
        // we asked for more input but did not consume what we already had
        TZ_FAIL(EXC_UNEXPECTED_SIGN_STATE);

    global.keys.apdu.sign.u.clear.total_length += cmd->lc;

    tz_parser_refill(st, cmd->data, cmd->lc);
    if (PKT_IS_LAST(cmd))
        tz_operation_parser_set_size(
            st, global.keys.apdu.sign.u.clear.total_length);
    TZ_CHECK(refill());
    tz_ui_stream();

    TZ_POSTAMBLE;
}

#ifdef HAVE_NBGL
static nbgl_layoutTagValueList_t useCaseTagValueList;
static nbgl_pageInfoLongPress_t  infoLongPress;

void
reject_blindsign_cb(void)
{
    FUNC_ENTER(("void"));

    stream_cb(TZ_UI_STREAM_CB_REJECT);
    ui_home_init();

    FUNC_LEAVE();
}

void
accept_blindsign_cb(void)
{
    FUNC_ENTER(("void"));

    stream_cb(TZ_UI_STREAM_CB_ACCEPT);
    ui_home_init();

    FUNC_LEAVE();
}

static void
reviewChoice(bool confirm)
{
    FUNC_ENTER(("confirm=%d", confirm));

    if (confirm) {
        nbgl_useCaseStatus("SIGNING\nSUCCESSFUL", true, accept_blindsign_cb);
    } else {
        nbgl_useCaseStatus("Rejected", false, reject_blindsign_cb);
    }

    FUNC_LEAVE();
}

static const char *transaction_type;
static char
    hash[TZ_BASE58_BUFFER_SIZE(sizeof(global.keys.apdu.hash.final_hash))];
static nbgl_layoutTagValue_t pair;
static nbgl_layoutTagValue_t *
getTagValuePair(uint8_t pairIndex)
{
    switch (pairIndex) {
    case 0:
        pair.item  = "Type";
        pair.value = transaction_type;
        break;
    case 1:
        pair.item  = "Hash";
        pair.value = hash;
        break;
    default:
        return NULL;
    }
    return &pair;
}

void
continue_blindsign_cb(void)
{
    FUNC_ENTER(("void"));

    useCaseTagValueList.pairs             = NULL;
    useCaseTagValueList.callback          = getTagValuePair;
    useCaseTagValueList.startIndex        = 0;
    useCaseTagValueList.nbPairs           = 2;
    useCaseTagValueList.smallCaseForValue = false;
    useCaseTagValueList.wrapping          = false;
    infoLongPress.icon                    = &C_tezos;
    infoLongPress.text                    = "Approve";
    infoLongPress.longPressText           = "Sign";
    nbgl_useCaseStaticReview(&useCaseTagValueList, &infoLongPress, "Reject",
                             reviewChoice);

    FUNC_LEAVE();
}

#endif

static void
handle_data_apdu_blind(command_t *cmd)
{
    TZ_PREAMBLE(("cmd=0x%p", cmd));

    TZ_ASSERT_NOTNULL(cmd);
    if (!global.keys.apdu.sign.u.blind.tag)
        global.keys.apdu.sign.u.blind.tag = cmd->data[0];
    if (!PKT_IS_LAST(cmd)) {
        io_send_sw(SW_OK);
        TZ_SUCCEED();
    }

    const char *type = "unknown type";

    global.keys.apdu.sign.received_last_msg = true;
    global.keys.apdu.sign.step              = SIGN_ST_WAIT_USER_INPUT;

    // clang-format off
    switch(global.keys.apdu.sign.u.blind.tag) {
    case 0x01: case 0x11: type = "Block\nproposal";         break;
    case 0x03:            type = "Manager\noperation";      break;
    case 0x02:
    case 0x12: case 0x13: type = "Consensus\noperation";    break;
    case 0x05:            type = "Micheline\nexpression";   break;
    default:                                                break;
    }
    // clang-format on

#ifdef HAVE_BAGL
    tz_ui_stream_push_all(TZ_UI_STREAM_CB_NOCB, "Sign Hash", type,
                          TZ_UI_LAYOUT_BNP, TZ_UI_ICON_NONE);

    tz_ui_stream();
#elif HAVE_NBGL
    char obuf[TZ_BASE58_BUFFER_SIZE(sizeof(FINAL_HASH))];
    if (tz_format_base58(FINAL_HASH, sizeof(FINAL_HASH), obuf, sizeof(obuf)))
        TZ_FAIL(EXC_UNKNOWN);

    char request[80];
    snprintf(request, sizeof(request), "Review request to blind\nsign %s",
             type);

    transaction_type = type;
    STRLCPY(hash, obuf);

    nbgl_useCaseReviewStart(&C_tezos, request, NULL, "Reject request",
                            continue_blindsign_cb, reject_blindsign_cb);
#endif

    /* XXXrcd: the logic here need analysis. */
    TZ_POSTAMBLE;
}
#undef FINAL_HASH

#ifdef HAVE_BAGL
#define GET_HOME_SCREEN() tz_ui_stream_get_cb_type()
#elif HAVE_NBGL
#define GET_HOME_SCREEN() global.home_screen
#endif

void
handle_apdu_sign(command_t *cmd)
{
    bool return_hash = cmd->ins == INS_SIGN_WITH_HASH;
    TZ_PREAMBLE(("cmd=0x%p", cmd));

    TZ_ASSERT(EXC_WRONG_LENGTH_FOR_INS, cmd->lc <= MAX_APDU_SIZE);

    if (PKT_IS_FIRST(cmd)) {
        TZ_ASSERT(EXC_UNEXPECTED_STATE, global.step == ST_IDLE);

        memset(&global.keys, 0, sizeof(global.keys));

        // clang-format off
        switch (GET_HOME_SCREEN()) {
        case SCREEN_CLEAR_SIGN: global.step = ST_CLEAR_SIGN; break;
        case SCREEN_BLIND_SIGN: global.step = ST_BLIND_SIGN; break;
        default:
            TZ_FAIL(EXC_UNEXPECTED_STATE);
        }
        // clang-format on

        TZ_CHECK(handle_first_apdu(cmd));
        global.keys.apdu.sign.return_hash = return_hash;
        goto end;
    }

    TZ_ASSERT(EXC_UNEXPECTED_STATE,
              global.step == ST_BLIND_SIGN || global.step == ST_CLEAR_SIGN);
    TZ_ASSERT(EXC_INVALID_INS,
              return_hash == global.keys.apdu.sign.return_hash);
    TZ_CHECK(handle_data_apdu(cmd));

    TZ_POSTAMBLE;
}
