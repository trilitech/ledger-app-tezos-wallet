/* Tezos Ledger application - Dynamic UI to display a stream of pages

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

#pragma once

/* This implements a multi-page screen, allowing to display a
   potentially infinite number of screens, keeping a bounded history.
   The user can query new screens using the right button, and go back
   a few screens using the left button (until history limit is
   reached).

   When a new page is needed, the display will call the `refill`
   callback, which in turn can call `tz_ui_stream_push` to add a new
   page. When the last page is reached, `tz_ui_stream_close` should be
   called, and the two final special pages to `accept` and `reject`
   the operation are pushed. The user can trigger the `accept` and
   `reject` callbacks by pressing both buttons while there pages are
   displayed.

   It is also possible to use this display engine for non streamed
   data by pushing a precomputed series of pages with
   `tz_ui_stream_push`, calling `tz_ui_stream_close`, and launching
   with a `refill` callback set to NULL. */

#include <stdbool.h>

#define TZ_UI_STREAM_HISTORY_SCREENS  4
#define TZ_UI_STREAM_TITLE_WIDTH      TZ_SCREEN_WITDH_BETWEEN_ICONS_BOLD_11PX
#define TZ_UI_STREAM_CONTENTS_WIDTH   TZ_SCREEN_WITDH_FULL_REGULAR_11PX
#define TZ_UI_STREAM_CONTENTS_LINES   (TZ_SCREEN_LINES_11PX - 1)

#define TZ_UI_STREAM_CONTENTS_SIZE (TZ_UI_STREAM_CONTENTS_WIDTH * TZ_UI_STREAM_CONTENTS_LINES)

typedef enum {
  TZ_UI_STREAM_CB_ACCEPT,
  TZ_UI_STREAM_CB_REFILL,
  TZ_UI_STREAM_CB_REJECT
} tz_ui_cb_type_t;

typedef struct {
  void (*cb)(tz_ui_cb_type_t);
  char titles[TZ_UI_STREAM_HISTORY_SCREENS][TZ_UI_STREAM_TITLE_WIDTH + 1];
  char values[TZ_UI_STREAM_HISTORY_SCREENS][TZ_UI_STREAM_CONTENTS_SIZE + 1];
  int16_t current;
  int16_t total;
  bool full;
} tz_ui_stream_t;

typedef enum {
  TZ_UI_STREAM_DISPLAY_INIT,
  TZ_UI_STREAM_DISPLAY_FIRST,
  TZ_UI_STREAM_DISPLAY_CANNOT_GO_BACK,
  TZ_UI_STREAM_DISPLAY_CONT,
  TZ_UI_STREAM_DISPLAY_ACCEPT,
  TZ_UI_STREAM_DISPLAY_REJECT,
} tz_ui_stream_screen_kind;

void tz_ui_stream_init(void (*)(tz_ui_cb_type_t));
/* Push title & content to screen
 *
 * content may not always fit on screen entirely - returns total
 * bytes of content written.
 */
size_t tz_ui_stream_push(const char *, const char *);
void tz_ui_stream_close(void);
tz_ui_stream_screen_kind tz_ui_stream_current_screen_kind(void);
__attribute__((noreturn)) void tz_ui_stream(void);
