/* Tezos Ledger application - Generic stream display

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>
   Copyright 2023 TriliTech <contact@trili.tech>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */
#ifdef HAVE_NBGL

#include <nbgl_use_case.h>
#include <ux.h>

#include "globals.h"
#include "ui_stream.h"

bool tz_ui_nav_cb(void);
bool has_final_screen(void);
void tz_ui_stream_start(void);
void tz_transaction_choice(bool getMorePairs);

void drop_last_screen(void);
void push_str(const char *text, size_t len, char **out);
void switch_to_blindsigning_on_error(void);

void
tz_reject(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));

    // Stax can reject early
    global.keys.apdu.sign.step              = SIGN_ST_WAIT_USER_INPUT;
    global.keys.apdu.sign.received_last_msg = true;

    if (global.step == ST_BLIND_SIGN) {
        s->cb(TZ_UI_STREAM_CB_BLINDSIGN_REJECT);
    } else {
        s->cb(TZ_UI_STREAM_CB_REJECT);
    }

    global.step = ST_IDLE;
    nbgl_useCaseReviewStatus(STATUS_TYPE_TRANSACTION_REJECTED, ui_home_init);
    FUNC_LEAVE();
}

void
tz_reject_ui(void)
{
    FUNC_ENTER(("void"));

    nbgl_useCaseConfirm("Reject transaction?", NULL, REJECT_CONFIRM_BUTTON,
                        RESUME("transaction"), tz_reject);

    FUNC_LEAVE();
}

static void
blindsign_choice(bool confirm)
{
    TZ_PREAMBLE(("void"));
    if (confirm) {
        tz_reject_ui();
    } else {
        tz_ui_stream_t *s = &global.stream;
        s->cb(TZ_UI_STREAM_CB_BLINDSIGN);
    }

    TZ_POSTAMBLE;
}

void
switch_to_blindsigning_on_error(void)
{
    TZ_PREAMBLE(("void"));
    TZ_ASSERT(EXC_UNEXPECTED_STATE, global.step == ST_CLEAR_SIGN);
    global.keys.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
    global.step                = ST_BLIND_SIGN;

    // copy error code
    tz_ui_stream_t *s        = &global.stream;
    size_t          bucket   = s->current % TZ_UI_STREAM_HISTORY_SCREENS;
    const char     *err_code = s->screens[bucket].pairs[0].value;
    PRINTF("[DEBUG] refill_error: global.step = %d\n %s", global.step,
           err_code);
    char blindsign_msg[150]
        = "Transaction could not be decoded correctly. Learn More:\n"
          "bit.ly/ledger-tez\nERROR: ";
    memcpy(blindsign_msg + strlen(blindsign_msg), err_code, ERROR_CODE_SIZE);

    // Show error msg and ask user to proceed to blindsign
    nbgl_useCaseChoice(&C_Important_Circle_64px,
                       "The transaction cannot be trusted", blindsign_msg,
                       "Reject transaction", "Proceed to Blindsign",
                       blindsign_choice);
    TZ_POSTAMBLE;
}

void
expert_mode_splash(void)
{
    TZ_PREAMBLE(("void"));

    nbgl_useCaseReviewStart(&C_Important_Circle_64px, "Expert mode",
                            "Next screen requires careful review",
                            "Reject transaction", tz_ui_stream_start,
                            tz_reject_ui);

    TZ_POSTAMBLE;
}

void
handle_expert_mode(bool confirm)
{
    TZ_PREAMBLE(("void"));
    if (confirm) {
        if (!N_settings.expert_mode) {
            toggle_expert_mode();
        }

        nbgl_useCaseStatus("EXPERT MODE\nENABLED", true, tz_ui_stream_start);

    } else {
        tz_reject_ui();
    }
    TZ_POSTAMBLE;
}

void
tz_enable_expert_mode_ui(void)
{
    FUNC_LEAVE();

    nbgl_useCaseChoice(&C_Important_Circle_64px,
                       "Enable expert mode to authorize this "
                       "transaction",
                       "", "Enable expert mode", "Reject transaction",
                       handle_expert_mode);

    FUNC_LEAVE();
}

void
tz_accept_ui(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));

    global.keys.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
    s->cb(TZ_UI_STREAM_CB_ACCEPT);

    nbgl_useCaseReviewStatus(STATUS_TYPE_TRANSACTION_SIGNED, ui_home_init);

    FUNC_LEAVE();
}

void
tz_choice_ui(bool accept)
{
    FUNC_ENTER(("accept=%d", accept));

    if (accept) {
        tz_accept_ui();
    } else {
        tz_reject();
    }

    FUNC_LEAVE();
}

void
tz_ui_continue(void)
{
    tz_ui_stream_t *s = &global.stream;

    TZ_PREAMBLE(("void"));

    if (!s->full) {
        TZ_CHECK(s->cb(TZ_UI_STREAM_CB_REFILL));
    }

    TZ_POSTAMBLE;
}

void
tz_ui_stream_cb(void)
{
    FUNC_ENTER(("void"));
    bool result = tz_ui_nav_cb();
    if (result) {
        tz_ui_stream_t         *s = &global.stream;
        tz_ui_stream_display_t *c = &s->current_screen;

        nbgl_useCaseReviewStreamingContinue(&c->list, tz_transaction_choice);
    }
    FUNC_LEAVE();
}

void
tz_ui_stream(void)
{
    tz_ui_stream_t *s = &global.stream;
    FUNC_ENTER(("void"));

    if (s->stream_cb) {
        s->stream_cb();
    }

    FUNC_LEAVE();
    return;
}

void
tz_ui_review_start(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));

    s->stream_cb = &tz_ui_stream_cb;
    tz_ui_stream();

    FUNC_LEAVE();
}

void
tz_transaction_choice(bool getMorePairs)
{
    FUNC_ENTER();
    if (getMorePairs) {
        // get more pairs
        tz_ui_review_start();
    } else {
        tz_reject();
    }

    FUNC_LEAVE();
}

void
tz_ui_stream_init(void (*cb)(tz_ui_cb_type_t cb_type))
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("cb=%p", cb));
    memset(s, 0x0, sizeof(*s));
    memset(global.error_code, '\0', sizeof(global.error_code));
    s->cb            = cb;
    s->full          = false;
    s->last          = 0;
    s->current       = -1;
    s->total         = -1;
    s->pressed_right = false;

    ui_strings_init();
    nbgl_operationType_t op_type = TYPE_TRANSACTION;
    nbgl_useCaseReviewStreamingStart(op_type, &C_tezos,
                                     "Review request to sign operation", NULL,
                                     tz_transaction_choice);

    FUNC_LEAVE();
}

void
tz_ui_stream_start(void)
{
    FUNC_ENTER(("void"));
    tz_ui_stream_cb();
    FUNC_LEAVE();
}

void
tz_ui_stream_close(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("full=%d", s->full));
    if (s->full) {
        PRINTF("trying to close already closed stream display");
        THROW(EXC_UNKNOWN);
    }
    s->full = true;

    FUNC_LEAVE();
}

bool
tz_ui_nav_cb(void)
{
    FUNC_ENTER(("void"));

    tz_ui_stream_t         *s      = &global.stream;
    tz_ui_stream_display_t *c      = &s->current_screen;
    bool                    result = true;
    tz_parser_state        *st = &global.keys.apdu.sign.u.clear.parser_state;

    // Continue receiving data from the apdu until s->full is true.
    while (((s->total < 0) || ((s->current == s->total) && !s->full))
           && (st->errno < TZ_ERR_INVALID_TAG)) {
        PRINTF("tz_ui_nav_cb: Looping...\n");
        tz_ui_continue();
        if (global.step == ST_ERROR) {
            break;
        } else if (global.keys.apdu.sign.step == SIGN_ST_WAIT_DATA) {
            result = false;
            break;
        }
    }

    if (s->full && has_final_screen()) {
        s->total++;
    }

    if (global.step == ST_ERROR) {
        global.step = ST_IDLE;
        ui_home_init();
        result = false;
    } else if (global.step != ST_CLEAR_SIGN) {
        result = false;
    } else if ((s->current == s->total) && s->full) {
        PRINTF("[DEBUG] signing...\n");
        result = false;
        nbgl_useCaseReviewStreamingFinish(SIGN("transaction"), tz_choice_ui);
    } else if (s->total >= 0) {
        if (s->current < s->total) {
            s->current++;
        }

        size_t bucket = s->current % TZ_UI_STREAM_HISTORY_SCREENS;

        switch (tz_ui_stream_get_cb_type()) {
        case TZ_UI_STREAM_CB_CANCEL:
            switch_to_blindsigning_on_error();
            result = false;
            break;
        case TZ_UI_STREAM_CB_EXPERT_MODE_ENABLE:
            tz_enable_expert_mode_ui();
            result = false;
            break;
        case TZ_UI_STREAM_CB_EXPERT_MODE_FIELD:
            expert_mode_splash();
            s->current--;
            s->screens[bucket].cb_type = TZ_UI_STREAM_CB_NOCB;
            result                     = false;
            break;
        default:
            c->list.pairs             = s->screens[bucket].pairs;
            c->list.callback          = NULL;
            c->list.startIndex        = 0;
            c->list.nbPairs           = s->screens[bucket].nb_pairs;
            c->list.smallCaseForValue = false;
            c->list.wrapping          = true;
            result                    = true;
            break;
        }
    }

    FUNC_LEAVE();
    return result;
}

bool
has_final_screen(void)
{
    tz_ui_stream_t *s  = &global.stream;
    size_t last_bucket = (s->total + 1) % TZ_UI_STREAM_HISTORY_SCREENS;
    return s->screens[last_bucket].nb_pairs > 0;
}

/* pushl mechanism
 * We use s->total differently to other devices
 * s->total     points to the last completed screen
 * s->total + 1 points to the screen under construction
 */
size_t
tz_ui_stream_pushl(tz_ui_cb_type_t cb_type, const char *title,
                   const char *value, ssize_t max,
                   __attribute__((unused)) tz_ui_layout_type_t layout_type,
                   __attribute__((unused)) tz_ui_icon_t        icon)
{
    tz_ui_stream_t *s = &global.stream;
    bool            push_to_next;
    bool            append    = false;
    size_t          max_pairs = (cb_type == TZ_UI_STREAM_CB_CANCEL)
                                    ? 1
                                    : NB_MAX_DISPLAYED_PAIRS_IN_REVIEW;

    FUNC_ENTER(("title=%s, value=%s", title, value));
    if (s->full) {
        PRINTF("trying to push in already closed stream display");
        THROW(EXC_UNKNOWN);
    }

#ifdef TEZOS_DEBUG
    int    prev_total   = s->total;
    int    prev_current = s->current;
    int    prev_last    = s->last;
    size_t i;
#endif

    int    bucket = (s->total + 1) % TZ_UI_STREAM_HISTORY_SCREENS;
    size_t idx    = s->screens[bucket].nb_pairs;
    size_t offset = 0;
    size_t length = strlen(value);

    if ((s->total >= 0)
        && ((s->current % TZ_UI_STREAM_HISTORY_SCREENS) == bucket)) {
        PRINTF(
            "[ERROR] PANIC!!!! Overwriting current screen, some bad things "
            "are happening\n");
    }

    if (((cb_type == TZ_UI_STREAM_CB_CANCEL)
         || (cb_type == TZ_UI_STREAM_CB_EXPERT_MODE_ENABLE)
         || (cb_type == TZ_UI_STREAM_CB_EXPERT_MODE_FIELD))
        && (idx > 0)) {
        PRINTF("[DEBUG] PUSH_TO_NEXT: %x\n", cb_type);
        push_to_next = true;
    } else {
        /* Are we continuing to construct or starting from scratch? */
        if (idx == NB_MAX_DISPLAYED_PAIRS_IN_REVIEW) {
            PRINTF("[ERROR] PANIC!!! we pushing to a screen that's full");
        }

        if ((idx > 0)
            && (0 == strcmp(title, s->screens[bucket].pairs[idx - 1].item))) {
            append = true;
        }

        if (!append) {
            push_str(title, strlen(title),
                     (char **)&s->screens[bucket].pairs[idx].item);
        }

        if (max != -1) {
            length = MIN(length, (size_t)max);
        }

        if (idx == 0) {
            s->screens[bucket].cb_type = cb_type;
        }

        char *out = NULL;

        if (append) {
            bool can_fit = false;
            ui_strings_can_fit(length, &can_fit);
            if (!can_fit) {
                drop_last_screen();
            }

            offset = ui_strings_append_last(&value[offset], length, &out);
            idx--;
        } else {
            push_str(&value[offset], length,
                     (char **)&s->screens[bucket].pairs[idx].value);
            offset = length;
        }

        /* Check that the whole screen fits on the page
         * if it doesn't, we need to pop this pair, and move
         * to the next screen.
         */
        nbgl_layoutTagValueList_t l = {.nbPairs = idx,
                                       .pairs   = s->screens[bucket].pairs,
                                       .smallCaseForValue = false,
                                       .wrapping          = true};

        /* This returns the number that can fit. Potentially we could
         * optimistically push all 4 rows, and then try?
         */
        uint8_t fit = nbgl_useCaseGetNbTagValuesInPage((uint8_t)(idx + 1), &l,
                                                       0, &push_to_next);
        PRINTF("[DEBUG] idx=%d fit=%d push_to_next=%d\n", idx, fit,
               push_to_next);
        /*
         * Dont push to next screen if the number of pairs is one.
         */
        push_to_next = (fit <= (uint8_t)idx);

        if (push_to_next) {
            /* We need to move to the next screen, retry */
            if (append && (offset > 0)) {
                ui_strings_drop_last(&out);
            } else {
                ui_strings_drop_last(
                    (char **)&s->screens[bucket].pairs[idx].value);
                ui_strings_drop_last(
                    (char **)&s->screens[bucket].pairs[idx].item);
            }

            offset = 0;
        }
    }

    if (push_to_next
        || (!append && (++(s->screens[bucket].nb_pairs) == max_pairs))
        || (append && (offset == 0))) {
        s->total++;
        if ((s->total > 0)
            && ((s->total % TZ_UI_STREAM_HISTORY_SCREENS)
                == (s->last % TZ_UI_STREAM_HISTORY_SCREENS))) {
            drop_last_screen();
        }
    }

#ifdef TEZOS_DEBUG
    PRINTF("[DEBUG] tz_ui_stream_pushl(%s, %s, %u)\n", title, value, max);
    PRINTF("[DEBUG]        bucket     %d\n", bucket);
    PRINTF("[DEBUG]        nb_pairs   %d\n", s->screens[bucket].nb_pairs);

    for (i = 0; i < s->screens[bucket].nb_pairs; i++) {
        PRINTF("[DEBUG]        item[%d]:   \"%s\"\n", i,
               s->screens[bucket].pairs[i].item);
        PRINTF("[DEBUG]        value[%d]:  \"%s\"\n", i,
               s->screens[bucket].pairs[i].value);
    }
    PRINTF("[DEBUG]        total:     %d -> %d\n", prev_total, s->total);
    PRINTF("[DEBUG]        current:   %d -> %d\n", prev_current, s->current);
    PRINTF("[DEBUG]        last:      %d -> %d\n", prev_last, s->last);
    PRINTF("[DEBUG]        offset:    %d\n", offset);
    PRINTF("[DEBUG]        cb:        %x\n", s->screens[bucket].cb_type);
#endif  // TEZOS_DEBUG

    FUNC_LEAVE();

    return offset;
}

void
drop_last_screen(void)
{
    tz_ui_stream_t *s      = &global.stream;
    size_t          bucket = s->last % TZ_UI_STREAM_HISTORY_SCREENS;
    size_t          i;

    TZ_PREAMBLE(("last: %d", s->last));

    for (i = 0; i < s->screens[bucket].nb_pairs; i++) {
        if (s->screens[bucket].pairs[i].item) {
            TZ_CHECK(
                ui_strings_drop((char **)&s->screens[bucket].pairs[i].item));
        }
        if (s->screens[bucket].pairs[i].value) {
            TZ_CHECK(
                ui_strings_drop((char **)&s->screens[bucket].pairs[i].value));
        }
    }
    s->screens[bucket].nb_pairs = 0;

    s->last++;

    TZ_POSTAMBLE;
}

#endif
