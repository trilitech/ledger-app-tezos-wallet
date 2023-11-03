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

void change_screen_right(void);
bool tz_ui_nav_cb(uint8_t, nbgl_pageContent_t *);

enum {
    TZ_UI_DETAILS_EXPAND = FIRST_USER_TOKEN,
};

void
tz_cancel_ui(void)
{
    tz_ui_stream_t *s                 = &global.stream;
    char            error_message[50] = "";

    FUNC_ENTER(("void"));

    size_t bucket = s->current % TZ_UI_STREAM_HISTORY_SCREENS;
    STRLCPY(error_message, s->screens[bucket].title);
    strlcat(error_message, "\n", sizeof(error_message));
    strlcat(error_message, s->screens[bucket].body[0], sizeof(error_message));

    s->cb(TZ_UI_STREAM_CB_CANCEL);

    global.step = ST_IDLE;
    nbgl_useCaseStatus(error_message, false, ui_home_init);

    FUNC_LEAVE();
}

void
tz_reject(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));

    // Stax can reject early
    global.apdu.sign.step              = SIGN_ST_WAIT_USER_INPUT;
    global.apdu.sign.received_last_msg = true;

    s->cb(TZ_UI_STREAM_CB_REJECT);

    global.step = ST_IDLE;
    nbgl_useCaseStatus("Rejected", false, ui_home_init);

    FUNC_LEAVE();
}

void
tz_reject_ui(void)
{
    FUNC_ENTER(("void"));

    nbgl_useCaseConfirm("Reject transaction?", NULL, "Yes, Reject",
                        "Go back to transaction", tz_reject);

    FUNC_LEAVE();
}

void
tz_accept_ui(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));

    global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
    s->cb(TZ_UI_STREAM_CB_ACCEPT);

    nbgl_useCaseStatus("SIGNING\nSUCCESSFUL", true, ui_home_init);

    FUNC_LEAVE();
}

void
tz_choice_ui(bool accept)
{
    FUNC_ENTER(("accept=%d", accept));

    if (accept) {
        tz_accept_ui();
    } else {
        tz_reject_ui();
    }

    FUNC_LEAVE();
}

void
tz_ui_continue(void)
{
    FUNC_ENTER(("void"));

    tz_ui_stream_t *s = &global.stream;

    if (!s->full)
        s->cb(TZ_UI_STREAM_CB_REFILL);

    FUNC_LEAVE();
    return;
}

void
tz_ui_stream_controls_cb(int token, __attribute__((unused)) uint8_t index)
{
    FUNC_ENTER(("token=%d, index=%d", token, index));

    if (token == TZ_UI_DETAILS_EXPAND) {
        char *title = "something";
        char *body
            = "cOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADE"
              "UPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMA"
              "DEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELY"
              "MADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETE"
              "LYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLE"
              "TELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMP"
              "LETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcO"
              "MPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUP"
              "cOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADE"
              "UPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMADEUPcOMPLETELYMA"
              "DEUPcOMPLETELYMADEUP";

        nbgl_useCaseViewDetails(title, body, false);
    }

    FUNC_LEAVE();
}

void
tz_ui_stream_cb(void)
{
    FUNC_ENTER(("void"));

    nbgl_useCaseRegularReview(0, /* init-page */
                              0, /* num pages, zero for no progress */
                              "Reject", tz_ui_stream_controls_cb,
                              tz_ui_nav_cb, tz_choice_ui);

    FUNC_LEAVE();
}

void
tz_ui_stream(void)
{
    tz_ui_stream_t *s = &global.stream;
    FUNC_ENTER(("void"));

    if (s->stream_cb)
        s->stream_cb();

    FUNC_LEAVE();
    return;
}

void
tz_ui_review_start()
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));

    s->stream_cb = &tz_ui_stream_cb;
    tz_ui_stream();

    FUNC_LEAVE();
}

void
tz_ui_stream_init(void (*cb)(uint8_t))
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("cb=%p", cb));
    memset(s, 0x0, sizeof(*s));
    s->cb      = cb;
    s->full    = false;
    s->current = -1;
    s->total   = -1;

    nbgl_useCaseReviewStart(&C_tezos, "Review request to sign operation",
                            NULL, "Reject request", tz_ui_review_start,
                            tz_reject_ui);

    FUNC_LEAVE();
}

static nbgl_layoutTagValue_t *
tz_ui_current_screen(__attribute__((unused)) uint8_t pairIndex)
{
    FUNC_ENTER(("pairIndex=%d", pairIndex));

    tz_ui_stream_t         *s = &global.stream;
    tz_ui_stream_display_t *c = &s->current_screen;

    PRINTF("[DEBUG] pressed_right=%d\n", s->pressed_right);

    if (s->current < s->total && s->pressed_right) {
        s->pressed_right = false;
    }

    c->title[0] = 0;
    c->body[0]  = 0;

    char *body
        = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
          "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBB"
          "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
          "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
          "BBBBBBBBBBBBBBF";

    size_t bucket = s->current % TZ_UI_STREAM_HISTORY_SCREENS;
    STRLCPY(c->title, s->screens[bucket].title);
    STRLCPY(c->body, s->screens[bucket].body[0]);

    c->pair.item = c->title;
    /* c->pair.value = c->body; */
    c->pair.value = body;

    PRINTF("show title=%s, body=%s from bucket=%d\n", c->title, c->body,
           bucket);
    FUNC_LEAVE();
    return &c->pair;
}

void
tz_ui_stream_close(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));
    if (s->full) {
        PRINTF("trying to close already closed stream display");
        THROW(EXC_UNKNOWN);
    }
    s->full = true;

    if (global.apdu.sign.u.clear.skip_to_sign) {
        tz_ui_stream();
    }

    FUNC_LEAVE();
}

bool
tz_ui_nav_cb(uint8_t page, nbgl_pageContent_t *content)
{
    FUNC_ENTER(("page=%d, content=%p", page, content));

    tz_ui_stream_t         *s = &global.stream;
    tz_ui_stream_display_t *c = &s->current_screen;

    if (s->total < 0) {
        return false;
    }

    change_screen_right();

    PRINTF("[DEBUG]: pressed_right=%d, current=%d, total=%d, full=%d\n",
           s->pressed_right, s->current, s->total, s->full);

    if (page == LAST_PAGE_FOR_REVIEW) {
        // skipped
        PRINTF("Skip requested");
        global.apdu.sign.u.clear.skip_to_sign = true;
        tz_ui_continue();
    }

    bool result = true;
    if (global.step == ST_ERROR) {
        // TODO: this is handled by change_screen_right except we disable it
        // to use it here. We should make ui_stream fully compatible with
        // exception.h
        global.step = ST_IDLE;
        ui_home_init();
        result = false;
    } else if (global.step != ST_CLEAR_SIGN && global.step != ST_BLIND_SIGN) {
        result = false;
    } else if (tz_ui_stream_get_type() == TZ_UI_STREAM_CB_CANCEL) {
        // We hit an error in the parsing workflow...
        tz_cancel_ui();
        result = false;
    } else if ((s->current == s->total) && s->full) {
        content->type                        = INFO_LONG_PRESS;
        content->infoLongPress.icon          = &C_tezos;
        content->infoLongPress.text          = "Sign";
        content->infoLongPress.longPressText = "Sign";
    } else if (page == LAST_PAGE_FOR_REVIEW) {
        nbgl_useCaseSpinner("Loading operation");
        result = false;
    } else {
        c->list.pairs             = NULL;
        c->list.callback          = tz_ui_current_screen;
        c->list.startIndex        = 0;
        c->list.nbPairs           = 1;
        c->list.smallCaseForValue = false;
        c->list.wrapping          = false;
        /* content->type         = TAG_VALUE_LIST; */
        /* content->tagValueList = c->list; */

        content->type                               = TAG_VALUE_DETAILS;
        content->tagValueDetails.detailsButtonText  = "More";
        content->tagValueDetails.detailsButtonIcon  = NULL;
        content->tagValueDetails.detailsButtonToken = TZ_UI_DETAILS_EXPAND;
        content->tagValueDetails.tagValueList.nbMaxLinesForValue
            = NB_MAX_LINES_IN_REVIEW;
        content->tagValueDetails.tagValueList.nbPairs = 1;
        content->tagValueDetails.tagValueList.pairs = tz_ui_current_screen(0);
        /* content->tagValueDetails.tagValueList.callback =
         * tz_ui_current_screen; */

        result = true;
    }

    FUNC_LEAVE();
    return result;
}

#endif
