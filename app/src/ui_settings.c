/* Tezos Ledger application - Home screen display

   Copyright 2023 TriliTech <contact@nomadic-labs.com>

   With code excerpts from:
    - Legacy Tezos app, Copyright 2019 Obsidian Systems
    - Legacy Tezos app, Copyright 2023 Ledger
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

static void cb(tz_ui_cb_type_t);

#define BLIND_SIGNING	0x01
#define BACK    	0x02


static void
cb(tz_ui_cb_type_t type)
{
    FUNC_ENTER(("type=%u\n", type));
    switch (type) {
    case BLIND_SIGNING:
        toggle_blindsigning();
        ui_settings_init();
        break;
    case BACK:
        ui_home_init();
        break;
    }
}

void
ui_settings_init(void)
{
    const char *bsigning = "DISABLED";

    FUNC_ENTER(("void"));

    if (N_settings.blindsigning)
        bsigning = "ENABLED";

    tz_ui_stream_init(cb);
    tz_ui_stream_push(BLIND_SIGNING, "Blind Signing", bsigning, TZ_UI_ICON_NONE);
    tz_ui_stream_push(BACK, "Back", "", TZ_UI_ICON_DASHBOARD);
    tz_ui_stream_close();
    tz_ui_stream_start();
    FUNC_LEAVE();
}
