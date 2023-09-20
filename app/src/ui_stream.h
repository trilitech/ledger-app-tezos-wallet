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

#define TZ_UI_STREAM_HISTORY_SCREENS 8
#define TZ_UI_STREAM_TITLE_WIDTH     TZ_SCREEN_WITDH_BETWEEN_ICONS_BOLD_11PX
#define TZ_UI_STREAM_CONTENTS_WIDTH  TZ_SCREEN_WITDH_FULL_REGULAR_11PX
#define TZ_UI_STREAM_CONTENTS_LINES  (TZ_SCREEN_LINES_11PX - 1)

#define TZ_UI_STREAM_CONTENTS_SIZE \
    (TZ_UI_STREAM_CONTENTS_WIDTH * TZ_UI_STREAM_CONTENTS_LINES)

/*
 * In the following structure, "type" is passed to our callback and
 * it can be used to determine which screen was displayed when both
 * buttons were pressed.
 *
 * If TZ_UI_STREAM_SCREEN_NOCB is specified, no callback will be called
 * when both buttons are pressed.
 *
 * If (type | TZ_UI_STREAM_SCREEN_MAINMASK) > 0 then we will return
 * to the main menu after the callback has been processed.  Developers
 * can use this in their own definitions.
 */

typedef uint8_t tz_ui_cb_type_t;
#define TZ_UI_STREAM_CB_NOCB     0x00
#define TZ_UI_STREAM_CB_REFILL   0xef
#define TZ_UI_STREAM_CB_MAINMASK 0xf0
#define TZ_UI_STREAM_CB_REJECT   0xfe
#define TZ_UI_STREAM_CB_ACCEPT   0xff

/*
 * The icons we used are generalised to allow for seamless Stax support
 */

typedef uint8_t tz_ui_icon_t;
#define TZ_UI_ICON_NONE      0x00
#define TZ_UI_ICON_TICK      0x01
#define TZ_UI_ICON_CROSS     0x02
#define TZ_UI_ICON_DASHBOARD 0x03
#define TZ_UI_ICON_SETTINGS  0x04

typedef struct {
    tz_ui_icon_t    icon;
    tz_ui_cb_type_t type;
    char            title[TZ_UI_STREAM_TITLE_WIDTH + 1];
    char body[TZ_UI_STREAM_CONTENTS_LINES][TZ_UI_STREAM_CONTENTS_WIDTH + 1];
} tz_ui_stream_screen_t;

typedef struct {
    void (*cb)(tz_ui_cb_type_t);
    tz_ui_stream_screen_t screens[TZ_UI_STREAM_HISTORY_SCREENS];
    int16_t               current;
    int16_t               total;
    bool                  full;
    // FIXME: workaround for issue with non-local control flow. Remove once
    // fixed see !66
    bool pressed_right;
} tz_ui_stream_t;

void tz_ui_stream_init(void (*)(tz_ui_cb_type_t));
/* Push title & content to screen
 *
 * content may not always fit on screen entirely - returns total
 * bytes of content written.
 */
size_t tz_ui_stream_push(tz_ui_cb_type_t, const char *, const char *,
                         tz_ui_icon_t);
size_t tz_ui_stream_pushl(tz_ui_cb_type_t, const char *, const char *,
                          ssize_t, tz_ui_icon_t);
size_t tz_ui_stream_push_all(tz_ui_cb_type_t, const char *, const char *,
                             tz_ui_icon_t);
void   tz_ui_stream_push_accept_reject(void);
void   tz_ui_stream_close(void);
void   tz_ui_stream(void);
void   tz_ui_stream_start(void);
tz_ui_cb_type_t tz_ui_stream_get_type(void);
