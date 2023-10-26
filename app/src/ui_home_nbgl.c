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

#ifdef HAVE_NBGL

#include <os.h>
#include <ux.h>

#include "app_main.h"
#include "globals.h"
#include "nbgl_use_case.h"

void tz_ui_home_redisplay(void);

static const char *const infoTypes[] = {"Version", "Developer"};
static const char *const infoContents[]
    = {APPVERSION, "Trilitech Kanvas Limited et al."};

enum {
    BLIND_SIGNING_TOKEN = FIRST_USER_TOKEN,
};

static nbgl_layoutSwitch_t switches[1];

static bool
navigation_cb_wallet(__attribute__((unused)) uint8_t page,
                     nbgl_pageContent_t             *content)
{
    switch (page) {
    case 0:
        content->type                   = INFOS_LIST;
        content->infosList.nbInfos      = 2;
        content->infosList.infoTypes    = infoTypes;
        content->infosList.infoContents = infoContents;
        break;
    case 1:
        switches[0] = (nbgl_layoutSwitch_t){
            .initState = N_settings.blindsigning ? ON_STATE : OFF_STATE,
            .text      = "Blind signing",
            .subText   = "Enable blindsigning",
            .token     = BLIND_SIGNING_TOKEN,
            .tuneId    = TUNE_TAP_CASUAL};

        content->type                    = SWITCHES_LIST;
        content->switchesList.nbSwitches = 1;
        content->switchesList.switches   = (nbgl_layoutSwitch_t *)switches;
        break;
    default:
        return false;
    }

    return true;
}

static void
controls_callback(int token, __attribute__((unused)) uint8_t index)
{
    switch (token) {
    case BLIND_SIGNING_TOKEN:
        toggle_blindsigning();

        if (!N_settings.blindsigning)
            global.home_screen = SCREEN_CLEAR_SIGN;
        break;
    }
}

static void
ui_menu_about_wallet(void)
{
    uint8_t nb_screens = 2;
    nbgl_useCaseSettings("Tezos wallet", 0, nb_screens, false,
                         tz_ui_home_redisplay, navigation_cb_wallet,
                         controls_callback);
}

static void
ui_toggle_clear_blind(void)
{
    // clang-format off
    switch (global.home_screen) {
    case SCREEN_CLEAR_SIGN: global.home_screen = SCREEN_BLIND_SIGN; break;
    case SCREEN_BLIND_SIGN: global.home_screen = SCREEN_CLEAR_SIGN; break;
    default:
        THROW (EXC_UNEXPECTED_STATE);
    }
    // clang-format on

    tz_ui_home_redisplay();
}

#define CLEAR_SIGN_HOME_TEXT \
    "This app enables signing transactions on the Tezos Network"
#define BLIND_SIGN_HOME_TEXT \
    "Ready for blind signing operations on the Tezos Network"

void
tz_ui_home_redisplay(void)
{
    FUNC_ENTER(("void"));

    if (!N_settings.blindsigning) {
        nbgl_useCaseHome("Tezos Wallet", &C_tezos, CLEAR_SIGN_HOME_TEXT, true,
                         ui_menu_about_wallet, app_exit);
    } else {
        const char *button_text;
        const char *tagline;

        switch (global.home_screen) {
        case SCREEN_CLEAR_SIGN:
            tagline     = CLEAR_SIGN_HOME_TEXT;
            button_text = "blind sign";
            break;
        case SCREEN_BLIND_SIGN:
            tagline     = BLIND_SIGN_HOME_TEXT;
            button_text = "clear sign";
            break;
        default:
            THROW(EXC_UNEXPECTED_STATE);
        }

        nbgl_useCaseHomeExt("Tezos Wallet", &C_tezos, tagline, true,
                            button_text, ui_toggle_clear_blind,
                            ui_menu_about_wallet, app_exit);
    }

    FUNC_LEAVE();
}

#endif  // HAVE_NBGL
