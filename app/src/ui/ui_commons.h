/* Tezos Ledger application - Some UI primitives

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 TriliTech <contact@trili.tech>

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

#pragma once

#include <os.h>
#include <os_io_seproxyhal.h>
#include <ux.h>

/**
 * @brief Callback for flow confirmation/rejection.
 *
 * @param confirm: whether the user accept or reject
 */
typedef void (*action_validate_cb)(bool confirm);

#ifdef HAVE_BAGL

/**
 * @brief Macro to display navigation icons and set associated callback.
 *
 */
#define DISPLAY(elts, cb, len)                                   \
    memcpy(global.ui.stream.current_screen.bagls, elts,          \
           len * sizeof(bagl_element_t));                        \
    G_ux.stack[0].element_arrays[0].element_array                \
        = global.ui.stream.current_screen.bagls;                 \
    G_ux.stack[0].element_arrays[0].element_array_count  = len;  \
    G_ux.stack[0].button_push_callback                   = cb;   \
    G_ux.stack[0].screen_before_element_display_callback = NULL; \
    UX_WAKE_UP();                                                \
    UX_REDISPLAY();

#define REGULAR BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER
#define BOLD    BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER
#endif  // HAVE_BAGL

#define SIGN_BUTTON           "Hold to sign"
#define REJECT_BUTTON         "Reject"
#define SIGN(msg)             "Sign " msg "?"
#define REVIEW(msg)           "Review " msg
#define REJECT(msg)           "Reject " msg
#define REJECT_QUESTION(msg)  REJECT(msg) "?"
#define REJECT_CONFIRM_BUTTON "Yes, reject"
#define RESUME(msg)           "Go back to " msg
