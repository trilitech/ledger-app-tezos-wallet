/* Tezos Ledger application - Generic stream display

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
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

#include "globals.h"
#include "nbgl_use_case.h"

void tz_reject(void) {
  tz_ui_stream_t *s = &global.stream;

  FUNC_ENTER(("void"));

  s->cb(TZ_UI_STREAM_CB_REJECT);
  ui_home_init();

  FUNC_LEAVE();
}

void tz_reject_ui(void) {

  // Stax can move into user input at any point in the flow
  global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;

  nbgl_useCaseStatus("Rejected", false, tz_reject);
}


void tz_accept_blindsign_cb(void) {
  tz_ui_stream_t *s = &global.stream;

  FUNC_ENTER(("void"));

  global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
  s->cb(TZ_UI_STREAM_CB_ACCEPT);
  ui_home_init();

  FUNC_LEAVE();
}

void tz_choice_ui(bool accept) {
  FUNC_ENTER(("accept=%d", accept));

  if (accept) {
    nbgl_useCaseStatus("SIGNING\nSUCCESSFUL", true, tz_accept_blindsign_cb);
  } else {
    tz_reject_ui();
  }

  FUNC_LEAVE();
}

void tz_ui_continue(void);
void tz_ui_layout_touch_cb(__attribute__((unused))int token, __attribute__((unused))uint8_t index) {
  FUNC_ENTER(("token=%d, index=%d", token, index));
  tz_ui_continue();
  FUNC_LEAVE();
}

bool tz_ui_nav_cb(uint8_t, nbgl_pageContent_t*);

static bool first = true;
static bool skipped = false;
void tz_ui_continue(void) {
  FUNC_ENTER(("void"));

  tz_ui_stream_t *s = &global.stream;

  if (!skipped)
    nbgl_useCaseForwardOnlyReview("Reject",
                                    tz_ui_layout_touch_cb,
                                    tz_ui_nav_cb,
                                    tz_choice_ui);

  if (first) {
    first = false;
    return;
  } else if (!s->full) {
        s->cb(TZ_UI_STREAM_CB_REFILL);
  }
  FUNC_LEAVE();
}


void tz_ui_stream_init (void (*cb)(uint8_t)) {
  tz_ui_stream_t *s = &global.stream;

  FUNC_ENTER(("cb=%p", cb));
  memset(s, 0x0, sizeof(*s));
  s->cb = cb;
  s->full = false;
  s->current = 0;
  s->total = -1;

  nbgl_useCaseReviewStart(&C_tezos,
                        "Review request to sign operation",
                        NULL,
                        "Reject request",
                        tz_ui_continue,
                        tz_reject_ui
                        );

  FUNC_LEAVE();
}

static char tz_ui_pair_content[80];
static nbgl_layoutTagValue_t tz_ui_pair;
static nbgl_layoutTagValueList_t tz_ui_tag_value_list;

static nbgl_layoutTagValue_t* tz_ui_getTagValuePair(__attribute__((unused))uint8_t pairIndex) {
  tz_ui_pair.value = tz_ui_pair_content;
  return &tz_ui_pair;
}

static bool tz_ui_next = false;

bool tz_ui_nav_cb(uint8_t page, nbgl_pageContent_t* content) {
  FUNC_ENTER((""));

  tz_ui_stream_t *s = &global.stream;
  if (page == LAST_PAGE_FOR_REVIEW) {
    // skipped
    PRINTF("Skip requested");
    skipped = true;
    global.apdu.sign.u.clear.skip_to_sign = true;

    //while (!s->full) {
    //    global.apdu.sign.step = SIGN_ST_WAIT_DATA;
    //    s->cb(TZ_UI_STREAM_CB_REFILL);
    //}

    //content->type = INFO_LONG_PRESS;
    //content->infoLongPress.icon = &C_tezos;
    //content->infoLongPress.text = "Sign";
    //content->infoLongPress.longPressText = "Sign";
  }

  if (tz_ui_next) {
    tz_ui_next = false;
    tz_ui_continue();
  }

  PRINTF("Left tz_ui_continue, full=%d\n", s->full);

  if (!s->full && !skipped) {
    tz_ui_tag_value_list.pairs = NULL;
    tz_ui_tag_value_list.callback = tz_ui_getTagValuePair;
    tz_ui_tag_value_list.startIndex = 0;
    tz_ui_tag_value_list.nbPairs = 1;
    tz_ui_tag_value_list.smallCaseForValue = false;
    tz_ui_tag_value_list.wrapping = false;

    content->type = TAG_VALUE_LIST;
    content->tagValueList = tz_ui_tag_value_list;

    tz_ui_next = true;
  } else {
    content->type = INFO_LONG_PRESS;
    content->infoLongPress.icon = &C_tezos;
    content->infoLongPress.text = "Sign";
    content->infoLongPress.longPressText = "Sign";
  }

  FUNC_LEAVE();

  return (!skipped || s->full);
}

size_t tz_ui_stream_push(tz_ui_cb_type_t type, const char *title,
                         const char *value, tz_ui_icon_t icon) {
  return tz_ui_stream_pushl(type, title, value, -1, icon);
}

size_t tz_ui_stream_push_all(__attribute__((unused))tz_ui_cb_type_t type,
                             __attribute__((unused))const char *title,
                             const char *value,
                             __attribute__((unused))tz_ui_icon_t icon) {
  return strlen(value);
}

size_t tz_ui_stream_pushl(__attribute__((unused))tz_ui_cb_type_t type,
                          const char *title,
                          const char *value, ssize_t max,
                          __attribute__((unused))tz_ui_icon_t icon) {

  FUNC_ENTER(("title=%s, value=%s", title, value));

  size_t length = strlen(value);

  if (max != -1)
    length = MIN(length, (size_t)max);

  tz_ui_pair.item  = title;
  strcpy(tz_ui_pair_content, value);
  return length;
}

__attribute__((noreturn)) void tz_ui_stream() {
  FUNC_ENTER(("void"));

  //tz_ui_stream_t *s = &global.stream;

  THROW(ASYNC_EXCEPTION);
}

#endif
