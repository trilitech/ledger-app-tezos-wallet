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
#define TZ_UI_STREAM_HISTORY_SCREENS \
    5  /// Max number of screens in history for nanos
#else
#define TZ_UI_STREAM_HISTORY_SCREENS \
    8   /// Max number of screens in history for nanos2/nanox.
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

/**
 * @brief Following #define's specify different "cb_types" which are passed to
 * our callback and it can be used to determine which screen was displayed
 * when both buttons were pressed.
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
#define TZ_UI_STREAM_CB_VALIDATE           0x0Fu
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
 * @brief Layout type enum
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
    TZ_UI_LAYOUT_BN      = 0x01,
    TZ_UI_LAYOUT_B       = 0x02,
    TZ_UI_LAYOUT_N       = 0x03,
    TZ_UI_LAYOUT_PB      = 0x04,
    TZ_UI_LAYOUT_HOME_PB = (TZ_UI_LAYOUT_HOME_MASK | TZ_UI_LAYOUT_PB),
    TZ_UI_LAYOUT_HOME_BN = (TZ_UI_LAYOUT_HOME_MASK | TZ_UI_LAYOUT_BN),
    TZ_UI_LAYOUT_HOME_B  = (TZ_UI_LAYOUT_HOME_MASK | TZ_UI_LAYOUT_B),
    TZ_UI_LAYOUT_HOME_N  = (TZ_UI_LAYOUT_HOME_MASK | TZ_UI_LAYOUT_N)
} tz_ui_layout_type_t;

/**
 * @brief The icons we used are generalised to allow for seamless Stax support
 */
typedef uint8_t tz_ui_icon_t;
#define TZ_UI_ICON_NONE      0x00
#define TZ_UI_ICON_TICK      0x01
#define TZ_UI_ICON_CROSS     0x02
#define TZ_UI_ICON_DASHBOARD 0x03
#define TZ_UI_ICON_SETTINGS  0x04
#define TZ_UI_ICON_BACK      0x05
#define TZ_UI_ICON_EYE       0x06
#define TZ_UI_ICON_WARNING   0x07

/**
 * @brief Represents a single screen's content and formatting for a ledger
 * device.
 *
 */
typedef struct {
    tz_ui_cb_type_t
        cb_type;  /// call back type for actions taken on this screen.
#ifdef HAVE_BAGL
    tz_ui_icon_t icon;  /// Icon to display on the screen.
    tz_ui_layout_type_t
        layout_type;   /// Layout type for the screen. CAN BP, BNP, NP, PB or
                       /// HOME_X where X can be one of the BP, BNP, PB.
    uint8_t body_len;  /// number of non-empty lines in the body.
    char   *title;     /// Title to display on the screen.
    char   *body[TZ_UI_STREAM_CONTENTS_LINES];  /// Body to display on the
                                                /// screen (Below title).
#else
    nbgl_layoutTagValue_t
        pairs[NB_MAX_DISPLAYED_PAIRS_IN_REVIEW];  /// Title-value pairs to be
                                                  /// displayed on stax
                                                  /// screen, Max 4 pairs can
                                                  /// be displayed on one
                                                  /// screen in stax.
    uint8_t nb_pairs;                             /// Number of pairs to be displayed on the stax screen.
#endif
} tz_ui_stream_screen_t;

#ifdef HAVE_NBGL
/**
 * @brief Holds list of title-value pairs for the current screen on stax.
 *
 */
typedef struct {
    nbgl_layoutTagValueList_t list;
} tz_ui_stream_display_t;
#endif  // HAVE_NBGL

/**
 * @brief Holds data for current and all the history screens.
 *
 */
typedef struct {
    void (*cb)(tz_ui_cb_type_t cb_type);
    tz_ui_stream_screen_t
        screens[TZ_UI_STREAM_HISTORY_SCREENS];  // array containing info of
                                                // all screens.
    tz_ui_strings_t strings;  // Ring buffer containing text data to be
                              // displayed on screen.
    int16_t current;          // index of current screen.
    int16_t total;            // total number of screens.
    int16_t last;             // index of last screen.
    bool    full;             // true if history is full.
    bool    pressed_right;    // true if right button was pressed.
#ifdef HAVE_NBGL
    tz_ui_stream_display_t
         current_screen;  // current screen's title-value pairs.
    char verify_address[TZ_BASE58CHECK_BUFFER_SIZE(
        20, 3)];  //  Holds the public key..
    nbgl_callback_t
        stream_cb;  // callback to be called when new screen is needed.
#endif              // HAVE_NBGL
} tz_ui_stream_t;

void tz_ui_stream_init(void (*cb)(tz_ui_cb_type_t cb_type));

/**
 * @brief  Push title & content to a single screen
 * content may not always fit on screen entirely - returns total
 * bytes of content written.
 *
 * @param cb_type callback type for the screen being pushed.
 * @param title title to be displayed
 * @param value text to be displayed.
 * @param layout_type Layout type, can be one of BP, BNP, NP, HOME_PB, ... and
 * so on.
 * @param icon icon to be displayed on the screen.
 * @return size_t  size of content written on the screen.(for ex. when value
 * is too large to fit on one screen, only part of it is written and rest is
 * displayed on next screen.)
 */
size_t tz_ui_stream_push(tz_ui_cb_type_t cb_type, const char *title,
                         const char *value, tz_ui_layout_type_t layout_type,
                         tz_ui_icon_t icon);

/**
 * @brief  internal implementation of tz_ui_stream_push, implemented
 * differently for stax and nano* devices.
 *
 * @param cb_type callback type for the screen being pushed.
 * @param title title to be displayed
 * @param value text to be displayed.
 * @param max max chars of value to be displayed. (default: -1)
 * @param layout_type Layout type, can be one of BP, BNP, NP, HOME_PB, ... and
 * so on.
 * @param icon icon to be displayed on the screen.
 * @return size_t  size of content written on the screen.(for ex. when value
 * is too large to fit on one screen, only part of it is written and rest is
 * displayed on next screen.)
 */
size_t tz_ui_stream_pushl(tz_ui_cb_type_t cb_type, const char *title,
                          const char *value, ssize_t max,
                          tz_ui_layout_type_t layout_type, tz_ui_icon_t icon);

/**
 * @brief  Push title- value pair, internally calls tz_ui_stream_push multiple
 * times so that entire value is pushed, even if it takes multiple screens.
 *
 * @param cb_type callback type
 * @param title Title to be displayed on the screen
 * @param value text to be displayed on the screen
 * @param layout_type Layout type
 * @param icon Icon to be displayed on the screen
 * @return size_t returns total number of characters written on the screen.
 */
size_t tz_ui_stream_push_all(tz_ui_cb_type_t cb_type, const char *title,
                             const char         *value,
                             tz_ui_layout_type_t layout_type,
                             tz_ui_icon_t        icon);

/**
 * @brief Indicates the stream is closed. Can not close it again.
 *
 */
void tz_ui_stream_close(void);

/**
 * @brief Redisplay the screen, called when additional data is pushed to the
 * screen or when user presses buttons.
 *
 */
void tz_ui_stream(void);

/**
 * @brief Start display of contents stored in ring buffer.
 *
 */
void tz_ui_stream_start(void);

/**
 * @brief Get the callback type for current screen.
 *
 * @return tz_ui_cb_type_t
 */
tz_ui_cb_type_t tz_ui_stream_get_cb_type(void);

#ifdef HAVE_NBGL
/**
 * @brief Send Reject code.
 *
 */
void tz_reject(void);
/**
 * @brief Reject confirmation screen.
 *
 */
void tz_reject_ui(void);
#endif
