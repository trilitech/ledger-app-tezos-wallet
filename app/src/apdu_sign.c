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

#ifdef HAVE_SWAP
#include <swap.h>
#endif

#include "apdu.h"
#include "apdu_sign.h"
#include "format.h"
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
static void send_reject(int error_code);
static void send_continue(void);
static void send_cancel(void);
static void refill(void);
static void refill_all(void);
static void stream_cb(tz_ui_cb_type_t cb_type);
static void handle_first_apdu(command_t *cmd);
static void handle_first_apdu_clear(command_t *cmd);
static void init_blind_stream(void);
static void handle_data_apdu(command_t *cmd);
static void handle_data_apdu_clear(command_t *cmd);
static void handle_data_apdu_blind(void);
#ifdef HAVE_BAGL
static void init_too_many_screens_stream(void);
static void init_summary_stream(void);
#endif

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

#define NB_MAX_SCREEN_ALLOWED 20
#define SCREEN_DISPLAYED      global.keys.apdu.sign.u.clear.screen_displayed

#ifdef HAVE_BAGL
void
tz_ui_stream_push_accept_reject(void)
{
    FUNC_ENTER(("void"));
#ifdef TARGET_NANOS
    tz_ui_stream_push(TZ_UI_STREAM_CB_ACCEPT, "Accept and send", "",
                      TZ_UI_LAYOUT_HOME_PB, TZ_UI_ICON_TICK);
#else
    tz_ui_stream_push(TZ_UI_STREAM_CB_ACCEPT, "Accept", "and send",
                      TZ_UI_LAYOUT_HOME_PB, TZ_UI_ICON_TICK);
#endif
    tz_ui_stream_push(TZ_UI_STREAM_CB_REJECT, "Reject", "",
                      TZ_UI_LAYOUT_HOME_PB, TZ_UI_ICON_CROSS);
    FUNC_LEAVE();
}

void
tz_ui_stream_push_risky_accept_reject(tz_ui_cb_type_t accept_cb_type,
                                      tz_ui_cb_type_t reject_cb_type)
{
    FUNC_ENTER(("void"));
    tz_ui_stream_push(accept_cb_type, "Accept risk", "", TZ_UI_LAYOUT_HOME_PB,
                      TZ_UI_ICON_TICK);
    tz_ui_stream_push(reject_cb_type, "Reject", "", TZ_UI_LAYOUT_HOME_PB,
                      TZ_UI_ICON_CROSS);
    FUNC_LEAVE();
}

void
tz_ui_stream_push_warning_not_trusted(
#ifdef TARGET_NANOS
    void
#else
    const char *title_reason, const char *value_reason
#endif
)
{
    FUNC_ENTER(("void"));
#ifdef TARGET_NANOS
    tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "The transaction",
                      "cannot be trusted.", TZ_UI_LAYOUT_HOME_B,
                      TZ_UI_ICON_NONE);
#else
    tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "The transaction",
                      "cannot be trusted.", TZ_UI_LAYOUT_HOME_PB,
                      TZ_UI_ICON_WARNING);
    tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, title_reason, value_reason,
                      TZ_UI_LAYOUT_HOME_N, TZ_UI_ICON_NONE);
    tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "It may not be safe",
                      "to sign this\ntransaction.", TZ_UI_LAYOUT_HOME_N,
                      TZ_UI_ICON_NONE);
#endif
    FUNC_LEAVE();
}

void
tz_ui_stream_push_learn_more(void)
{
    FUNC_ENTER(("void"));
    tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB,
                      "Learn More:", "bit.ly/ledger-tez",
                      TZ_UI_LAYOUT_HOME_BN, TZ_UI_ICON_NONE);
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

    /* If we aren't returning the hash, zero its buffer. */
    if (!global.keys.apdu.sign.return_hash) {
        memset((void *)bufs[0].ptr, 0, bufs[0].size);
        bufs[0].size = 0;
    }

    io_send_response_buffers(bufs, 2, SW_OK);
    global.step = ST_IDLE;

    TZ_POSTAMBLE;
}

static void
send_reject(int error_code)
{
    TZ_PREAMBLE(("void"));

    APDU_SIGN_ASSERT_STEP(SIGN_ST_WAIT_USER_INPUT);
    TZ_FAIL(error_code);
    TZ_POSTAMBLE;
}

static void
send_continue(void)
{
    TZ_PREAMBLE(("void"));

    APDU_SIGN_ASSERT((global.keys.apdu.sign.step == SIGN_ST_WAIT_USER_INPUT)
                     || (global.keys.apdu.sign.step == SIGN_ST_WAIT_DATA));
    APDU_SIGN_ASSERT(!global.keys.apdu.sign.received_last_msg);

    if (global.keys.apdu.sign.u.clear.received_msg) {
        global.keys.apdu.sign.u.clear.received_msg = false;
        io_send_sw(SW_OK);
    }

    global.keys.apdu.sign.step = SIGN_ST_WAIT_DATA;

    TZ_POSTAMBLE;
}

static void
refill_blo_im_full(void)
{
    tz_parser_state *st    = &global.keys.apdu.sign.u.clear.parser_state;
    size_t           wrote = 0;
    TZ_PREAMBLE(("void"));

    // No display for Swap or Summary flow
    if (
#ifdef HAVE_SWAP
        G_called_from_swap ||
#endif
        global.step == ST_SUMMARY_SIGN) {
        tz_parser_flush(st, global.line_buf, TZ_UI_STREAM_CONTENTS_SIZE);
        // invoke refill until we consume entire msg.
        TZ_SUCCEED();
    }

    global.keys.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
#ifdef HAVE_BAGL
    if (st->field_info.is_field_complex && !N_settings.expert_mode) {
        tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, st->field_info.field_name,
                          "Needs Expert mode", TZ_UI_LAYOUT_HOME_B,
                          TZ_UI_ICON_NONE);
        tz_ui_stream_push(TZ_UI_STREAM_CB_REJECT, "Home", "",
                          TZ_UI_LAYOUT_HOME_PB, TZ_UI_ICON_BACK);
        tz_ui_stream_close();
        goto end;
    } else {
        if (st->field_info.is_field_complex
            && !global.keys.apdu.sign.u.clear.displayed_expert_warning) {
            SCREEN_DISPLAYED++;
            tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Next field requires",
                              "careful review", TZ_UI_LAYOUT_HOME_B,
                              TZ_UI_ICON_NONE);
            global.keys.apdu.sign.u.clear.last_field_index
                = st->field_info.field_index;
            global.keys.apdu.sign.u.clear.displayed_expert_warning = true;
        }
    }

    if ((N_settings.blindsign_status == ST_BLINDSIGN_LARGE_TX)
        && (SCREEN_DISPLAYED >= NB_MAX_SCREEN_ALLOWED)) {
        init_too_many_screens_stream();
        TZ_SUCCEED();
    } else {
        SCREEN_DISPLAYED++;
        wrote = tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB,
                                  st->field_info.field_name, global.line_buf,
                                  TZ_UI_LAYOUT_BN, TZ_UI_ICON_NONE);
    }

#elif HAVE_NBGL
    PRINTF("[DEBUG] field=%s complex=%d\n", st->field_info.field_name,
           st->field_info.is_field_complex);
    if (st->field_info.is_field_complex
        && !global.keys.apdu.sign.u.clear.displayed_expert_warning) {
        global.keys.apdu.sign.u.clear.last_field_index
            = st->field_info.field_index;
        global.keys.apdu.sign.u.clear.displayed_expert_warning = true;
        if (!N_settings.expert_mode) {
            tz_ui_stream_push_all(TZ_UI_STREAM_CB_EXPERT_MODE_ENABLE,
                                  st->field_info.field_name, "complex",
                                  TZ_UI_LAYOUT_BN, TZ_UI_ICON_NONE);
        }

        wrote = tz_ui_stream_push_all(
            TZ_UI_STREAM_CB_EXPERT_MODE_FIELD, st->field_info.field_name,
            global.line_buf, TZ_UI_LAYOUT_BN, TZ_UI_ICON_NONE);
    } else {
        wrote = tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB,
                                  st->field_info.field_name, global.line_buf,
                                  TZ_UI_LAYOUT_BN, TZ_UI_ICON_NONE);
    }

#endif
    tz_parser_flush_up_to(st, global.line_buf, TZ_UI_STREAM_CONTENTS_SIZE,
                          wrote);
    TZ_POSTAMBLE;
}

static void
refill_blo_done(void)
{
    tz_parser_state *st = &global.keys.apdu.sign.u.clear.parser_state;
    TZ_PREAMBLE(("void"));

    TZ_ASSERT(
        EXC_UNEXPECTED_STATE,
        (global.keys.apdu.sign.received_last_msg && st->regs.ilen) == 0);

    global.keys.apdu.sign.u.clear.received_msg = false;
    if (st->regs.oofs != 0) {
        refill_blo_im_full();
        TZ_SUCCEED();
    }

    global.keys.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
    if (global.step == ST_SWAP_SIGN) {
        TZ_CHECK(sign_packet());
        TZ_SUCCEED();
    }

#ifdef HAVE_BAGL
    if (global.step == ST_SUMMARY_SIGN) {
        TZ_CHECK(init_summary_stream());
        TZ_SUCCEED();
    }

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

    global.keys.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
#ifdef HAVE_SWAP
    if (G_called_from_swap) {
        global.keys.apdu.sign.u.clear.received_msg = false;
        TZ_FAIL(EXC_PARSE_ERROR);
    }
#endif

    // clang-format off
#ifdef HAVE_BAGL
    tz_ui_stream_init(stream_cb);

    tz_ui_stream_push_warning_not_trusted(
#ifndef TARGET_NANOS
            "This transaction", "could not be\ndecoded correctly."
#endif
    );

    tz_ui_stream_push_all(TZ_UI_STREAM_CB_NOCB,
                          "Parsing error",
                          tz_parser_result_name(st->errno),
                          TZ_UI_LAYOUT_HOME_BN,
                          TZ_UI_ICON_NONE);
    tz_ui_stream_push_learn_more();
    tz_ui_stream_push_risky_accept_reject(TZ_UI_STREAM_CB_BLINDSIGN, TZ_UI_STREAM_CB_CANCEL);

#elif HAVE_NBGL
    // The following call is just to invoke switch_to_blindsigning
    // with TZ_UI_STREAM_CB_CANCEL callback type in function tz_ui_nav_cb()
    // The text will not be shown.
    tz_ui_stream_push_all(TZ_UI_STREAM_CB_CANCEL,
                          "Parsing error",
                          tz_parser_result_name(st->errno),
                          TZ_UI_LAYOUT_BN,
                          TZ_UI_ICON_NONE);
#endif
    // clang-format on

    tz_ui_stream_close();

#ifdef HAVE_BAGL
    tz_ui_stream_start();
#endif

    TZ_POSTAMBLE;
}

static void
refill(void)
{
    tz_parser_state *st = &global.keys.apdu.sign.u.clear.parser_state;
    TZ_PREAMBLE(("void"));

    while (!TZ_IS_BLOCKED(tz_operation_parser_step(st))) {
        // Loop while the result is successful and not blocking
    }
    PRINTF("[DEBUG] refill(errno: %s)\n", tz_parser_result_name(st->errno));
    // clang-format off
    switch (st->errno) {
    case TZ_BLO_IM_FULL: TZ_CHECK(refill_blo_im_full()); break;
    case TZ_BLO_FEED_ME: TZ_CHECK(send_continue());      break;
    case TZ_BLO_DONE:    TZ_CHECK(refill_blo_done());    break;
    default:             TZ_CHECK(refill_error());       break;
    }
    // clang-format on
    TZ_POSTAMBLE;
}

/**
 * @brief Parse until there is nothing left to parse or user input is
 * required.
 */
static void
refill_all(void)
{
    TZ_PREAMBLE(("void"));

    while (global.keys.apdu.sign.u.clear.received_msg) {
        TZ_CHECK(refill());
        if ((global.step == ST_SUMMARY_SIGN)
            && (global.keys.apdu.sign.step == SIGN_ST_WAIT_USER_INPUT)) {
            break;
        }
    }

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
pass_from_clear_to_blind(void)
{
    TZ_PREAMBLE(("void"));

    APDU_SIGN_ASSERT_STEP(SIGN_ST_WAIT_USER_INPUT);

    global.step                        = ST_BLIND_SIGN;
    global.keys.apdu.sign.step         = SIGN_ST_WAIT_DATA;
    global.keys.apdu.sign.u.blind.step = BLINDSIGN_ST_OPERATION;

    init_blind_stream();
    handle_data_apdu_blind();

    TZ_POSTAMBLE;
}

static void
stream_cb(tz_ui_cb_type_t cb_type)
{
    TZ_PREAMBLE(("cb_type=%u", cb_type));

    // clang-format off
    switch (cb_type) {
    case TZ_UI_STREAM_CB_ACCEPT:             TZ_CHECK(sign_packet());              break;
    case TZ_UI_STREAM_CB_REFILL:             TZ_CHECK(refill());                   break;
    case TZ_UI_STREAM_CB_REJECT:             send_reject(EXC_REJECT);              break;
    case TZ_UI_STREAM_CB_BLINDSIGN_REJECT:   send_reject(EXC_PARSE_ERROR);         break;
    case TZ_UI_STREAM_CB_CANCEL:             TZ_CHECK(send_cancel());              break;
    case TZ_UI_STREAM_CB_BLINDSIGN:          TZ_CHECK(pass_from_clear_to_blind()); break;
    default:                                 TZ_FAIL(EXC_UNKNOWN);                 break;
    }
    // clang-format on

    TZ_POSTAMBLE;
}

#ifdef HAVE_BAGL
static void
push_next_summary_screen(void)
{
#define FINAL_HASH       global.keys.apdu.hash.final_hash
#define SUMMARYSIGN_STEP global.keys.apdu.sign.u.summary.step

    char num_buffer[TZ_DECIMAL_BUFFER_SIZE(TZ_NUM_BUFFER_SIZE / 8)] = {0};
    char hash_buffer[TZ_BASE58_BUFFER_SIZE(sizeof(FINAL_HASH))]     = {0};
    tz_operation_state *op
        = &global.keys.apdu.sign.u.clear.parser_state.operation;

    TZ_PREAMBLE(("void"));

    switch (SUMMARYSIGN_STEP) {
    case SUMMARYSIGN_ST_OPERATION:
        SUMMARYSIGN_STEP = SUMMARYSIGN_ST_NB_TX;

        snprintf(num_buffer, sizeof(num_buffer), "%d", op->batch_index);
        tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Number of Tx", num_buffer,
                          TZ_UI_LAYOUT_BN, TZ_UI_ICON_NONE);
        break;
    case SUMMARYSIGN_ST_NB_TX:
        SUMMARYSIGN_STEP = SUMMARYSIGN_ST_AMOUNT;

        tz_mutez_to_string(num_buffer, sizeof(num_buffer), op->total_amount);
        tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Total amount", num_buffer,
                          TZ_UI_LAYOUT_BN, TZ_UI_ICON_NONE);
        break;
    case SUMMARYSIGN_ST_AMOUNT:
        SUMMARYSIGN_STEP = SUMMARYSIGN_ST_FEE;

        tz_mutez_to_string(num_buffer, sizeof(num_buffer), op->total_fee);
        tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Total fee", num_buffer,
                          TZ_UI_LAYOUT_BN, TZ_UI_ICON_NONE);
        break;
    case SUMMARYSIGN_ST_FEE:
        SUMMARYSIGN_STEP = SUMMARYSIGN_ST_HASH;

        if (tz_format_base58(FINAL_HASH, sizeof(FINAL_HASH), hash_buffer,
                             sizeof(hash_buffer))) {
            TZ_FAIL(EXC_UNKNOWN);
        }
        tz_ui_stream_push_all(TZ_UI_STREAM_CB_NOCB, "Hash", hash_buffer,
                              TZ_UI_LAYOUT_BN, TZ_UI_ICON_NONE);
        break;
    case SUMMARYSIGN_ST_HASH:
        SUMMARYSIGN_STEP = SUMMARYSIGN_ST_ACCEPT_REJECT;

        tz_ui_stream_push_accept_reject();
        tz_ui_stream_close();
        break;
    default:
        PRINTF("Unexpected summary state: %d\n", (int)SUMMARYSIGN_STEP);
        TZ_FAIL(EXC_UNEXPECTED_STATE);
    };

    TZ_POSTAMBLE;

#undef SUMMARYSIGN_STEP
#undef FINAL_HASH
}

static void
summary_stream_cb(tz_ui_cb_type_t cb_type)
{
    TZ_PREAMBLE(("cb_type=%u", cb_type));

    // clang-format off
    switch (cb_type) {
    case TZ_UI_STREAM_CB_ACCEPT: TZ_CHECK(sign_packet());              break;
    case TZ_UI_STREAM_CB_REJECT: send_reject(EXC_REJECT);              break;
    case TZ_UI_STREAM_CB_REFILL: TZ_CHECK(push_next_summary_screen()); break;
    default:                     TZ_FAIL(EXC_UNKNOWN);                 break;
    }
    // clang-format on

    TZ_POSTAMBLE;
}

static void
init_summary_stream(void)
{
    TZ_PREAMBLE(("void"));

    tz_ui_stream_init(summary_stream_cb);

    global.keys.apdu.sign.u.summary.step = SUMMARYSIGN_ST_OPERATION;
    push_next_summary_screen();

    tz_ui_stream();

    TZ_POSTAMBLE;
}

static void
pass_from_clear_to_summary(void)
{
    TZ_PREAMBLE(("void"));

    APDU_SIGN_ASSERT_STEP(SIGN_ST_WAIT_USER_INPUT);

    global.step                = ST_SUMMARY_SIGN;
    global.keys.apdu.sign.step = SIGN_ST_WAIT_DATA;

    TZ_CHECK(refill_all());

    TZ_POSTAMBLE;
}

static void
too_many_screens_stream_cb(tz_ui_cb_type_t cb_type)
{
    TZ_PREAMBLE(("cb_type=%u", cb_type));

    // clang-format off
    switch (cb_type) {
    case TZ_UI_STREAM_CB_VALIDATE: TZ_CHECK(pass_from_clear_to_summary()); break;
    case TZ_UI_STREAM_CB_REJECT:   send_reject(EXC_REJECT);                break;
    default:                       TZ_FAIL(EXC_UNKNOWN);                   break;
    }
    // clang-format on

    TZ_POSTAMBLE;
}

static void
init_too_many_screens_stream(void)
{
    tz_ui_stream_init(too_many_screens_stream_cb);

    tz_ui_stream_push_warning_not_trusted(
#ifndef TARGET_NANOS
        "Operation too long", "Proceed to\nblindsign."
#endif
    );
#ifdef TARGET_NANOS
    tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Operation too long",
                      "Accept blindsign.", TZ_UI_LAYOUT_HOME_B,
                      TZ_UI_ICON_NONE);
#endif
    tz_ui_stream_push_risky_accept_reject(TZ_UI_STREAM_CB_VALIDATE,
                                          TZ_UI_STREAM_CB_REJECT);

    tz_ui_stream_close();
}
#endif

#define FINAL_HASH global.keys.apdu.hash.final_hash
#ifdef HAVE_BAGL
static void
bs_push_next(void)
{
#define BLINDSIGN_STEP global.keys.apdu.sign.u.blind.step

    char obuf[TZ_BASE58_BUFFER_SIZE(sizeof(FINAL_HASH))];

    TZ_PREAMBLE(("void"));

    switch (BLINDSIGN_STEP) {
    case BLINDSIGN_ST_OPERATION:
        BLINDSIGN_STEP = BLINDSIGN_ST_HASH;

        if (tz_format_base58(FINAL_HASH, sizeof(FINAL_HASH), obuf,
                             sizeof(obuf))) {
            TZ_FAIL(EXC_UNKNOWN);
        }

        tz_ui_stream_push_all(TZ_UI_STREAM_CB_NOCB, "Sign Hash", obuf,
                              TZ_UI_LAYOUT_BN, TZ_UI_ICON_NONE);
        break;
    case BLINDSIGN_ST_HASH:
        BLINDSIGN_STEP = BLINDSIGN_ST_ACCEPT_REJECT;

        tz_ui_stream_push_accept_reject();
        tz_ui_stream_close();
        break;
    default:
        PRINTF("Unexpected blindsign state: %d\n", (int)BLINDSIGN_STEP);
        TZ_FAIL(EXC_UNEXPECTED_STATE);
    };

    TZ_POSTAMBLE;

#undef BLINDSIGN_STEP
}

static void
bs_stream_cb(tz_ui_cb_type_t cb_type)
{
    TZ_PREAMBLE(("cb_type=%u", cb_type));

    // clang-format off
    switch (cb_type) {
    case TZ_UI_STREAM_CB_ACCEPT: return sign_packet();
    case TZ_UI_STREAM_CB_REFILL: return bs_push_next();
    case TZ_UI_STREAM_CB_REJECT: return send_reject(EXC_REJECT);
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

    TZ_LIB_CHECK(read_bip32_path(&global.path_with_curve.bip32_path,
                                 cmd->data, cmd->lc));
    global.path_with_curve.derivation_type = cmd->p2;
    TZ_ASSERT(EXC_WRONG_PARAM,
              check_derivation_type(global.path_with_curve.derivation_type));
    CX_CHECK(cx_blake2b_init_no_throw(&global.keys.apdu.hash.state,
                                      SIGN_HASH_SIZE * 8));
    /*
     * We set the tag to zero here which indicates that it is unset.
     * The first data packet will set it to the first byte.
     */
    global.keys.apdu.sign.tag = 0;

    TZ_CHECK(handle_first_apdu_clear(cmd));

    TZ_ASSERT(EXC_UNEXPECTED_STATE, (global.step == ST_CLEAR_SIGN)
                                        || (global.step == ST_SWAP_SIGN));

    io_send_sw(SW_OK);
    global.keys.apdu.sign.step = SIGN_ST_WAIT_DATA;

    TZ_POSTAMBLE;
}

static void
handle_first_apdu_clear(__attribute__((unused)) command_t *cmd)
{
    TZ_PREAMBLE(("global.step=%d", global.step));
    tz_parser_state *st = &global.keys.apdu.sign.u.clear.parser_state;

    global.keys.apdu.sign.u.clear.received_msg = false;
    // NO ui display during swap.
#ifdef HAVE_SWAP
    if (!G_called_from_swap) {
#endif
        tz_ui_stream_init(stream_cb);
        global.step = ST_CLEAR_SIGN;

#ifdef TARGET_NANOS
        tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Review operation", "",
                          TZ_UI_LAYOUT_HOME_PB, TZ_UI_ICON_EYE);
#elif defined(HAVE_BAGL)
    tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Review", "operation",
                      TZ_UI_LAYOUT_HOME_PB, TZ_UI_ICON_EYE);
#endif
#ifdef HAVE_SWAP
    } else {
        PRINTF("[DEBUG] If called from SWAP : global.step =%d\n",
               global.step);
    }
#endif
    tz_operation_parser_init(st, TZ_UNKNOWN_SIZE, false);
    tz_parser_refill(st, NULL, 0);
    tz_parser_flush(st, global.line_buf, TZ_UI_STREAM_CONTENTS_SIZE);

    TZ_POSTAMBLE;
}

static void
init_blind_stream(void)
{
#ifdef HAVE_BAGL
    tz_ui_stream_init(bs_stream_cb);
#elif HAVE_NBGL
    nbgl_useCaseSpinner("Loading operation");
#endif
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

    if (PKT_IS_LAST(cmd)) {
        global.keys.apdu.sign.received_last_msg = true;
    }

    if (!global.keys.apdu.sign.tag) {
        global.keys.apdu.sign.tag = cmd->data[0];
    }

    if ((N_settings.blindsign_status == ST_BLINDSIGN_ON)
        && (global.step == ST_CLEAR_SIGN)) {
        global.step = ST_SUMMARY_SIGN;
    }

    // clang-format off
    switch (global.step) {
    case ST_CLEAR_SIGN:
    case ST_SWAP_SIGN:
    case ST_SUMMARY_SIGN:
                          TZ_CHECK(handle_data_apdu_clear(cmd)); break;
    case ST_BLIND_SIGN:   TZ_CHECK(handle_data_apdu_blind());    break;
    default:              TZ_FAIL(EXC_UNEXPECTED_STATE);         break;
    }
    // clang-format on

    TZ_POSTAMBLE;
}

static void
handle_data_apdu_clear(command_t *cmd)
{
    tz_parser_state *st = &global.keys.apdu.sign.u.clear.parser_state;
    TZ_PREAMBLE(("cmd=0x%p", cmd));

    global.keys.apdu.sign.u.clear.received_msg = true;

    TZ_ASSERT_NOTNULL(cmd);
    if (st->regs.ilen > 0) {
        // we asked for more input but did not consume what we already had
        TZ_FAIL(EXC_UNEXPECTED_SIGN_STATE);
    }

    global.keys.apdu.sign.u.clear.total_length += cmd->lc;

    tz_parser_refill(st, cmd->data, cmd->lc);
    if (PKT_IS_LAST(cmd)) {
        tz_operation_parser_set_size(
            st, global.keys.apdu.sign.u.clear.total_length);
    }

    switch (global.step) {
    case ST_CLEAR_SIGN:
        TZ_CHECK(refill());
        if (global.keys.apdu.sign.step == SIGN_ST_WAIT_USER_INPUT) {
            tz_ui_stream();
        }
        break;
    case ST_SWAP_SIGN:
    case ST_SUMMARY_SIGN:
        TZ_CHECK(refill_all());
        break;
    default:
        TZ_FAIL(EXC_UNEXPECTED_SIGN_STATE);
        break;
    }

    TZ_POSTAMBLE;
}

#ifdef HAVE_NBGL
static nbgl_layoutTagValueList_t useCaseTagValueList;

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
        nbgl_useCaseStatus("TRANSACTION\nSIGNED", true, accept_blindsign_cb);
    } else {
        tz_reject();
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
    nbgl_operationType_t op = TYPE_TRANSACTION;

    useCaseTagValueList.pairs             = NULL;
    useCaseTagValueList.callback          = getTagValuePair;
    useCaseTagValueList.startIndex        = 0;
    useCaseTagValueList.nbPairs           = 2;
    useCaseTagValueList.smallCaseForValue = false;
    useCaseTagValueList.wrapping          = false;
    nbgl_useCaseReviewBlindSigning(op, &useCaseTagValueList, &C_tezos,
                                   REVIEW("Transaction"), NULL,
                                   SIGN("Transaction"), NULL, reviewChoice);

    FUNC_LEAVE();
}

#endif

static void
handle_data_apdu_blind(void)
{
    TZ_PREAMBLE(("void"));

    if (!global.keys.apdu.sign.received_last_msg) {
        io_send_sw(SW_OK);
        TZ_SUCCEED();
    }

    const char *type = "unknown type";

    global.keys.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;

    // clang-format off
    switch(global.keys.apdu.sign.tag) {
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
                          TZ_UI_LAYOUT_BN, TZ_UI_ICON_NONE);

    tz_ui_stream();
#elif HAVE_NBGL
    char obuf[TZ_BASE58_BUFFER_SIZE(sizeof(FINAL_HASH))];
    if (tz_format_base58(FINAL_HASH, sizeof(FINAL_HASH), obuf,
                         sizeof(obuf))) {
        TZ_FAIL(EXC_UNKNOWN);
    }

    transaction_type = type;
    STRLCPY(hash, obuf);
    continue_blindsign_cb();
#endif
    TZ_POSTAMBLE;
}
#undef FINAL_HASH

void
handle_apdu_sign(command_t *cmd)
{
    bool return_hash = cmd->ins == INS_SIGN_WITH_HASH;
    TZ_PREAMBLE(("cmd=0x%p", cmd));

    TZ_ASSERT(EXC_WRONG_LENGTH_FOR_INS, cmd->lc <= MAX_APDU_SIZE);

    if (PKT_IS_FIRST(cmd)) {
        TZ_ASSERT(EXC_UNEXPECTED_STATE,
                  (global.step == ST_IDLE) || (global.step == ST_SWAP_SIGN));

        memset(&global.keys, 0, sizeof(global.keys));
        TZ_CHECK(handle_first_apdu(cmd));
        global.keys.apdu.sign.return_hash = return_hash;
        goto end;
    }

    TZ_ASSERT(EXC_UNEXPECTED_STATE, (global.step == ST_BLIND_SIGN)
                                        || (global.step == ST_CLEAR_SIGN)
                                        || (global.step == ST_SUMMARY_SIGN)
                                        || (global.step == ST_SWAP_SIGN));
    TZ_ASSERT(EXC_INVALID_INS,
              return_hash == global.keys.apdu.sign.return_hash);
    TZ_CHECK(handle_data_apdu(cmd));

    TZ_POSTAMBLE;
}
