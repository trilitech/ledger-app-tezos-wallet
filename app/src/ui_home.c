/* Tezos Ledger application - Home screen display

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

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

#include "globals.h"
#include "ui_home_nbgl.h"

#ifdef HAVE_BAGL
/* Prototypes */

static void cb(tz_ui_cb_type_t);


static void cb(tz_ui_cb_type_t type) {
  FUNC_ENTER(("type=%u\n", type));

  switch (type) {
  case SCREEN_BLIND_SIGN:
  case SCREEN_CLEAR_SIGN:
    break;
  case SCREEN_SETTINGS:
    ui_settings_init();
    break;
  case SCREEN_QUIT:
    exit_app();
  }

  FUNC_LEAVE();
}

#ifdef TARGET_NANOS
static void clear_sign_screen(void) {
  tz_ui_stream_push(SCREEN_CLEAR_SIGN, "ready for",
                    "safe signing", TZ_UI_ICON_NONE);
}

static void blind_sign_screen(void) {
  tz_ui_stream_push(SCREEN_BLIND_SIGN, "ready for",
                    "BLIND signing", TZ_UI_ICON_NONE);
}
#else
static void clear_sign_screen(void) {
  tz_ui_stream_push(SCREEN_CLEAR_SIGN, "Tezos Wallet",
                    "ready for\nsafe signing", TZ_UI_ICON_NONE);
}

static void blind_sign_screen(void) {
  tz_ui_stream_push(SCREEN_BLIND_SIGN, "Tezos Wallet",
                    "ready for\nBLIND signing", TZ_UI_ICON_NONE);
}
#endif
#endif // HAVE_BAGL

void ui_home_init(void) {
  FUNC_ENTER(("void"));
#ifdef HAVE_BAGL
  tz_ui_stream_init(cb);
  clear_sign_screen();
  if (global.settings.blindsigning)
    blind_sign_screen();
  tz_ui_stream_push(SCREEN_SETTINGS, "Settings", "", TZ_UI_ICON_SETTINGS);
  tz_ui_stream_push(SCREEN_QUIT, "Quit?", "", TZ_UI_ICON_DASHBOARD);
  tz_ui_stream_close();
  tz_ui_stream_start();
#elif defined(HAVE_NBGL)
  tz_ui_home_redisplay();
#endif

  FUNC_LEAVE();
}
