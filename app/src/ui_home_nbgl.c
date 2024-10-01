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

static void controls_callback(int                             token,
                              __attribute__((unused)) uint8_t index,
                              __attribute__((unused)) int     page);
void        tz_ui_home_redisplay(uint8_t page);

//  -----------------------------------------------------------
//  --------------------- SETTINGS MENU -----------------------
//  -----------------------------------------------------------
#define SETTING_INFO_NB      3
#define SETTINGS_SWITCHES_NB 1
#define SETTINGS_RADIO_NB    2
static const char *const infoTypes[]    = {"Version", "Developer", "Contact"};
static const char *const infoContents[] = {
    APPVERSION, "Trilitech Kanvas Limited et al.", "ledger-tezos@trili.tech"};

enum {
    EXPERT_MODE_TOKEN = FIRST_USER_TOKEN,
    BLINDSIGN_MODE_TOKEN
};
enum {
    EXPERT_MODE_TOKEN_ID = 0,
    BLINDSIGN_MODE_TOKEN_ID,
    SETTINGS_CONTENTS_NB
};
enum {
    EXPERT_MODE_PAGE = 0,
    BLINDSIGN_PAGE   = 1
};

static nbgl_contentSwitch_t         expert_mode_switch = {0};
static const nbgl_contentInfoList_t infoList = {.nbInfos   = SETTING_INFO_NB,
                                                .infoTypes = infoTypes,
                                                .infoContents = infoContents};

static const char *const blindsign_choices_text[]
    = {"Blindsigning OFF", "Blindsigning ON"};

static void
get_contents(uint8_t index, nbgl_content_t *content)
{
    FUNC_ENTER(("Index: %d", index));
    if (index == EXPERT_MODE_TOKEN_ID) {
        content->content.switchesList.nbSwitches = SETTINGS_SWITCHES_NB;
        content->content.switchesList.switches   = &expert_mode_switch;
        content->type                            = SWITCHES_LIST;
        content->contentActionCallback           = controls_callback;
    } else {
        content->content.choicesList.nbChoices  = SETTINGS_RADIO_NB;
        content->content.choicesList.names      = blindsign_choices_text;
        content->content.choicesList.token      = BLINDSIGN_MODE_TOKEN;
        content->content.choicesList.initChoice = N_settings.blindsign_status;
        content->type                           = CHOICES_LIST;
        content->contentActionCallback          = controls_callback;
    }
    FUNC_LEAVE();
}

static void
controls_callback(int token, __attribute__((unused)) uint8_t index,
                  __attribute__((unused)) int page)
{
    FUNC_ENTER(("Token : %d, Index:  %d, Page: %d", token, index, page));
    uint8_t switch_value;
    if (token == EXPERT_MODE_TOKEN) {
        switch_value = !N_settings.expert_mode;
        toggle_expert_mode();
        expert_mode_switch.initState = (nbgl_state_t)(switch_value);
    }
    if (token == BLINDSIGN_MODE_TOKEN) {
        blindsign_state_t blindsign_status = (blindsign_state_t)(index % 2);
        set_blindsign_status(blindsign_status);
        tz_ui_home_redisplay(BLINDSIGN_PAGE);
    }
    FUNC_LEAVE();
}

#define HOME_TEXT "This app enables signing transactions on the Tezos Network"

void
initSettings(void)
{
    expert_mode_switch.initState = (nbgl_state_t)(N_settings.expert_mode);
    expert_mode_switch.text      = "Expert mode";
    expert_mode_switch.subText   = "Enable expert mode signing";
    expert_mode_switch.token     = EXPERT_MODE_TOKEN;
    expert_mode_switch.tuneId    = TUNE_TAP_CASUAL;
}

void
tz_ui_home_redisplay(uint8_t page)
{
    FUNC_ENTER(("void"));

    initSettings();
    static nbgl_genericContents_t tezos_settingContents = {0};
    tezos_settingContents.callbackCallNeeded            = false;
    tezos_settingContents.nbContents = SETTINGS_CONTENTS_NB;

    static nbgl_content_t contents[SETTINGS_CONTENTS_NB] = {0};
    get_contents(EXPERT_MODE_TOKEN_ID, &contents[EXPERT_MODE_TOKEN_ID]);
    get_contents(BLINDSIGN_MODE_TOKEN_ID, &contents[BLINDSIGN_MODE_TOKEN_ID]);

    tezos_settingContents.contentsList = contents;

    PRINTF("Entered settings and initialized\n");

    nbgl_useCaseHomeAndSettings("Tezos Wallet", &C_tezos, HOME_TEXT, page,
                                &tezos_settingContents, &infoList, NULL,
                                app_exit);

    FUNC_LEAVE();
}

#endif  // HAVE_NBGL
