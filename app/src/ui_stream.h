/* Tezos Ledger application - Dynamic UI to display a stream of pages

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

#ifdef HAVE_NBGL
#include <nbgl_use_case.h>
#endif

#include <stdbool.h>

#include "ui_strings.h"

#ifdef TARGET_NANOS
#define TZ_UI_STREAM_HISTORY_SCREENS 5
#else
#define TZ_UI_STREAM_HISTORY_SCREENS 8
#endif  // TARGET_NANOS

#define TZ_UI_STREAM_TITLE_WIDTH TZ_SCREEN_WITDH_BETWEEN_ICONS_BOLD_11PX

#ifdef HAVE_BAGL
#define TZ_UI_STREAM_CONTENTS_WIDTH TZ_SCREEN_WITDH_FULL_REGULAR_11PX
#define TZ_UI_STREAM_CONTENTS_LINES (TZ_SCREEN_LINES_11PX - 1)
#elif HAVE_NBGL
#define TZ_UI_STREAM_CONTENTS_WIDTH 20
#define TZ_UI_STREAM_CONTENTS_LINES 1
#endif

#define TZ_UI_STREAM_CONTENTS_SIZE \
    (TZ_UI_STREAM_CONTENTS_WIDTH * TZ_UI_STREAM_CONTENTS_LINES)

/*
 * In the following structure, "cb_type" is passed to our callback and
 * it can be used to determine which screen was displayed when both
 * buttons were pressed.
 *
 * If TZ_UI_STREAM_SCREEN_NOCB is specified, no callback will be called
 * when both buttons are pressed.
 *
 * If (cb_type | TZ_UI_STREAM_SCREEN_MAINMASK) > 0 then we will return
 * to the main menu after the callback has been processed.  Developers
 * can use this in their own definitions.
 */

typedef uint8_t tz_ui_cb_type_t;
#define TZ_UI_STREAM_CB_NOCB               0x00u
#define TZ_UI_STREAM_CB_BLINDSIGN          0x0Eu
#define TZ_UI_STREAM_CB_REFILL             0xEFu
#define TZ_UI_STREAM_CB_MAINMASK           0xF0u
#define TZ_UI_STREAM_CB_EXPERT_MODE_FIELD  0xFAu
#define TZ_UI_STREAM_CB_EXPERT_MODE_ENABLE 0xFBu
#define TZ_UI_STREAM_CB_BLINDSIGN_REJECT   0xFCu
#define TZ_UI_STREAM_CB_CANCEL             0xFDu
#define TZ_UI_STREAM_CB_REJECT             0xFEu
#define TZ_UI_STREAM_CB_ACCEPT             0xFFu

#define TZ_UI_LAYOUT_HOME_MASK 0x80u
/**
 * Layout type:
 * BNP - refers to Bold Title, normal text/picture below the title.
 * BP  - refers to Bold tile and picture below the title(optional).
 * NP  - Normal text and picture below the text(optional)
 * PB  - Picture and Bold title below it,left/rigth arrow all vertically
 *       centered.
 * HOME_X - X layout for home/settings screens
 * with left/right arrows vertically centered.
 *
 */
typedef enum : uint8_t {
    TZ_UI_LAYOUT_BNP      = 0x01,
    TZ_UI_LAYOUT_BP       = 0x02,
    TZ_UI_LAYOUT_NP       = 0x03,
    TZ_UI_LAYOUT_HOME_PB  = (0x04 | TZ_UI_LAYOUT_HOME_MASK),
    TZ_UI_LAYOUT_HOME_BNP = (0x05 | TZ_UI_LAYOUT_HOME_MASK),
    TZ_UI_LAYOUT_HOME_BP  = (0x06 | TZ_UI_LAYOUT_HOME_MASK),
    TZ_UI_LAYOUT_HOME_NP  = (0x07 | TZ_UI_LAYOUT_HOME_MASK)
} tz_ui_layout_type_t;

/*
 * The icons we used are generalised to allow for seamless Stax support
 */

typedef uint8_t tz_ui_icon_t;
#define TZ_UI_ICON_NONE      0x00
#define TZ_UI_ICON_TICK      0x01
#define TZ_UI_ICON_CROSS     0x02
#define TZ_UI_ICON_DASHBOARD 0x03
#define TZ_UI_ICON_SETTINGS  0x04
#define TZ_UI_ICON_BACK      0x05
#define TZ_UI_ICON_EYE       0x06

typedef struct {
    tz_ui_cb_type_t cb_type;
#ifdef HAVE_BAGL
    tz_ui_icon_t        icon;
    tz_ui_layout_type_t layout_type;
    char               *title;
    char               *body[TZ_UI_STREAM_CONTENTS_LINES];
#else
    nbgl_layoutTagValue_t pairs[NB_MAX_DISPLAYED_PAIRS_IN_REVIEW];
    uint8_t               nb_pairs;
#endif
} tz_ui_stream_screen_t;

#ifdef HAVE_NBGL
typedef struct {
    nbgl_layoutTagValueList_t list;
} tz_ui_stream_display_t;
#endif  // HAVE_NBGL

typedef struct {
    void (*cb)(tz_ui_cb_type_t cb_type);
    tz_ui_stream_screen_t screens[TZ_UI_STREAM_HISTORY_SCREENS];
    tz_ui_strings_t       strings;
    int16_t               current;
    int16_t               total;
    int16_t               last;
    bool                  full;
    // FIXME: workaround for issue with non-local control flow. Remove once
    // fixed see !66
    bool pressed_right;
#ifdef HAVE_NBGL
    tz_ui_stream_display_t current_screen;
    char                   verify_address[TZ_BASE58CHECK_BUFFER_SIZE(20, 3)];
    nbgl_callback_t        stream_cb;
#endif  // HAVE_NBGL
} tz_ui_stream_t;

void tz_ui_stream_init(void (*cb)(tz_ui_cb_type_t cb_type));
/* Push title & content to screen
 *
 * content may not always fit on screen entirely - returns total
 * bytes of content written.
 */
size_t tz_ui_stream_push(tz_ui_cb_type_t cb_type, const char *title,
                         const char *value, tz_ui_layout_type_t layout_type,
                         tz_ui_icon_t icon);
size_t tz_ui_stream_pushl(tz_ui_cb_type_t cb_type, const char *title,
                          const char *value, ssize_t max,
                          tz_ui_layout_type_t layout_type, tz_ui_icon_t icon);
size_t tz_ui_stream_push_all(tz_ui_cb_type_t cb_type, const char *title,
                             const char         *value,
                             tz_ui_layout_type_t layout_type,
                             tz_ui_icon_t        icon);
void   tz_ui_stream_close(void);
void   tz_ui_stream(void);
void   tz_ui_stream_start(void);
tz_ui_cb_type_t tz_ui_stream_get_cb_type(void);
#ifdef HAVE_NBGL
void tz_reject_ui(void);
#endif
