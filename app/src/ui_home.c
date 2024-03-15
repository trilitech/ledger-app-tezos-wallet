/* Tezos Ledger application - Home screen display

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Trilitech <contact@trili.tech>

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

#include "app_main.h"
#include "globals.h"
#include "ui_home_nbgl.h"

#ifdef HAVE_BAGL
/* Prototypes */

static void cb(tz_ui_cb_type_t cb_type);

static void
cb(tz_ui_cb_type_t cb_type)
{
    FUNC_ENTER(("cb_type=%u", cb_type));

    switch (cb_type) {
    case SCREEN_HOME:
    case SCREEN_VERSION:
        break;
    case SCREEN_SETTINGS:
        ui_settings_init(SETTINGS_HOME_PAGE);
        break;
    case SCREEN_QUIT:
        app_exit();
    }

    FUNC_LEAVE();
}

#endif  // HAVE_BAGL

void
ui_home_init(void)
{
    FUNC_ENTER(("void"));
#ifdef HAVE_BAGL
    tz_ui_stream_init(cb);
    tz_ui_stream_push(SCREEN_HOME, "Application", "is ready",
                      TZ_UI_LAYOUT_HOME_NP, TZ_UI_ICON_NONE);
    tz_ui_stream_push(SCREEN_VERSION, "Version", APPVERSION,
                      TZ_UI_LAYOUT_HOME_BNP, TZ_UI_ICON_NONE);
    tz_ui_stream_push(SCREEN_SETTINGS, "Settings", "", TZ_UI_LAYOUT_HOME_PB,
                      TZ_UI_ICON_SETTINGS);
    tz_ui_stream_push(SCREEN_QUIT, "Quit?", "", TZ_UI_LAYOUT_HOME_PB,
                      TZ_UI_ICON_DASHBOARD);
    tz_ui_stream_close();
    tz_ui_stream_start();
#elif defined(HAVE_NBGL)
    tz_ui_home_redisplay();
#endif

    FUNC_LEAVE();
}
