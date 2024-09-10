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
#include "os.h"
#include "ux.h"

#ifdef HAVE_BAGL
#include "glyphs.h"

UX_STEP_NOCB(ux_menu_ready_step, pnn,
             {&C_tezos_16px, "Application", "is ready"});
UX_STEP_NOCB(ux_menu_version_step, bn, {"Version", APPVERSION});
UX_STEP_CB(ux_menu_settings_step, pb, ui_settings_init(SETTINGS_HOME_PAGE),
           {&C_icon_coggle, "Settings"});
UX_STEP_CB(ux_menu_exit_step, pb, app_exit(), {&C_icon_dashboard_x, "Quit"});

// FLOW for the main menu:
// #1 screen: ready
// #2 screen: version of the app
// #3 screen: Settings menu
// #4 screen: quit
UX_FLOW(ux_menu_main_flow, &ux_menu_ready_step, &ux_menu_version_step,
        &ux_menu_settings_step, &ux_menu_exit_step, FLOW_LOOP);

void
ui_home_init(void)
{
    FUNC_ENTER(("void"));
    // reserve a display stack slot if none yet
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    ux_flow_init(0, ux_menu_main_flow, NULL);

    FUNC_LEAVE();
}
#endif  // HAVE_BAGL

#ifdef HAVE_NBGL
void
ui_home_init(void)
{
    FUNC_ENTER(("void"));
    tz_ui_home_redisplay(INIT_HOME_PAGE);
    FUNC_LEAVE();
}
#endif  // HAVE_NBGL
