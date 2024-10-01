/* Tezos Ledger application - Home screen display

   Copyright 2023 TriliTech <contact@trili.tech>

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

#ifdef HAVE_NBGL

#include <os.h>
#include <ux.h>

#include "app_main.h"
#include "globals.h"
#include "nbgl_use_case.h"

void tz_ui_home_redisplay(void);

//  -----------------------------------------------------------
//  --------------------- SETTINGS MENU -----------------------
//  -----------------------------------------------------------
#define SETTING_INFO_NB 3

static const char *const infoTypes[]    = {"Version", "Developer", "Contact"};
static const char *const infoContents[] = {
    APPVERSION, "Trilitech Kanvas Limited et al.", "ledger-tezos@trili.tech"};

enum {
    EXPERT_MODE_TOKEN = FIRST_USER_TOKEN,
    BLIND_SIGNING_TOKEN
};
enum {
    EXPERT_MODE_TOKEN_ID = 0,
    BLIND_SIGNING_TOKEN_ID,
    SETTINGS_SWITCHES_NB
};

static nbgl_layoutSwitch_t switches[SETTINGS_SWITCHES_NB] = {0};

static const nbgl_contentInfoList_t infoList = {.nbInfos   = SETTING_INFO_NB,
                                                .infoTypes = infoTypes,
                                                .infoContents = infoContents};

static void
controls_callback(int token, __attribute__((unused)) uint8_t index,
                  __attribute__((unused)) int page)
{
    uint8_t switch_value;
    if (token == BLIND_SIGNING_TOKEN) {
        switch_value = !N_settings.blindsigning;
        toggle_blindsigning();
        switches[BLIND_SIGNING_TOKEN_ID].initState
            = (nbgl_state_t)(switch_value);
    } else if (token == EXPERT_MODE_TOKEN) {
        switch_value = !N_settings.expert_mode;
        toggle_expert_mode();
        switches[EXPERT_MODE_TOKEN_ID].initState
            = (nbgl_state_t)(switch_value);
    }
}

#define SETTINGS_CONTENTS_NB 1
static const nbgl_content_t contentsList[SETTINGS_CONTENTS_NB] = {
    {.content.switchesList.nbSwitches = SETTINGS_SWITCHES_NB,
     .content.switchesList.switches   = switches,
     .type                            = SWITCHES_LIST,
     .contentActionCallback           = controls_callback}
};

static const nbgl_genericContents_t tezos_settingContents
    = {.callbackCallNeeded = false,
       .contentsList       = contentsList,
       .nbContents         = SETTINGS_CONTENTS_NB};
;

#define HOME_TEXT "This app enables signing transactions on the Tezos Network"
void
initSettings(void)
{
    switches[EXPERT_MODE_TOKEN_ID].initState
        = (nbgl_state_t)(N_settings.expert_mode);
    switches[EXPERT_MODE_TOKEN_ID].text    = "Expert mode";
    switches[EXPERT_MODE_TOKEN_ID].subText = "Enable expert mode signing";
    switches[EXPERT_MODE_TOKEN_ID].token   = EXPERT_MODE_TOKEN;
    switches[EXPERT_MODE_TOKEN_ID].tuneId  = TUNE_TAP_CASUAL;

    switches[BLIND_SIGNING_TOKEN_ID].initState
        = (nbgl_state_t)(N_settings.blindsigning);
    switches[BLIND_SIGNING_TOKEN_ID].text = "Blind signing";
    switches[BLIND_SIGNING_TOKEN_ID].subText
        = "Enable transaction blind signing";
    switches[BLIND_SIGNING_TOKEN_ID].token  = BLIND_SIGNING_TOKEN;
    switches[BLIND_SIGNING_TOKEN_ID].tuneId = TUNE_TAP_CASUAL;
}

void
tz_ui_home_redisplay(void)
{
    FUNC_ENTER(("void"));

    initSettings();

    nbgl_useCaseHomeAndSettings("Tezos Wallet", &C_tezos, HOME_TEXT,
                                INIT_HOME_PAGE, &tezos_settingContents,
                                &infoList, NULL, app_exit);

    FUNC_LEAVE();
}

#endif  // HAVE_NBGL
