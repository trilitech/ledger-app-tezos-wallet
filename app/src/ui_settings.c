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

#ifdef HAVE_BAGL
#include "globals.h"

static void
expert_mode_toggle()
{
    FUNC_ENTER();
    toggle_expert_mode();
    ui_settings_init(SETTINGS_HOME_PAGE);
    FUNC_LEAVE();
}

static void
blindsign_toggle()
{
    FUNC_ENTER();
    toggle_blindsign_status();
    ui_settings_init(SETTINGS_BLINDSIGN_PAGE);
    FUNC_LEAVE();
}

UX_STEP_CB(ux_expert_mode_step, bn, expert_mode_toggle(),
           {"Expert mode", global.expert_mode_state});
UX_STEP_CB(ux_blindsign_step, bn, blindsign_toggle(),
           {"Allow Blindsigning", global.blindsign_state_desc});
UX_STEP_CB(ux_back_step, pb, ui_home_init(), {&C_icon_back, "Back"});

UX_FLOW(ux_expert_mode_flow, &ux_expert_mode_step, &ux_blindsign_step,
        &ux_back_step, FLOW_LOOP);

void
ui_settings_init(int16_t page)
{
    FUNC_ENTER(("%d, Expert Mode: %d, Max_Screen: ", page,
                N_settings.expert_mode, N_settings.blindsign_status));

    if (N_settings.expert_mode) {
        strncpy(global.expert_mode_state, "ENABLED",
                sizeof(global.expert_mode_state));
    } else {
        strncpy(global.expert_mode_state, "DISABLED",
                sizeof(global.expert_mode_state));
    }

    switch (N_settings.blindsign_status) {
    case ST_BLINDSIGN_LARGE_TX:
    default:
        strncpy(global.blindsign_state_desc, "For Large Tx",
                sizeof(global.blindsign_state_desc));
        break;
    case ST_BLINDSIGN_ON:
        strncpy(global.blindsign_state_desc, "ON",
                sizeof(global.blindsign_state_desc));
        break;
    case ST_BLINDSIGN_OFF:
        strncpy(global.blindsign_state_desc, "OFF",
                sizeof(global.blindsign_state_desc));
        break;
    }
    if (page == SETTINGS_HOME_PAGE) {
        ux_flow_init(0, ux_expert_mode_flow, &ux_expert_mode_step);
    } else if (page == SETTINGS_BLINDSIGN_PAGE) {
        ux_flow_init(0, ux_expert_mode_flow, &ux_blindsign_step);
    }
    FUNC_LEAVE();
}

#endif
