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

#include "globals.h"
#include "nbgl_use_case.h"

static const char* const infoTypes[] = {"Version", "Developer", "Copyright"};
static const char* const infoContents[] = {VERSION, "Tezos", "(c) 2023 <Tezos>"};

static bool navigation_cb_wallet(__attribute__((unused))uint8_t page, nbgl_pageContent_t* content) {
    if (page == 0) {
        content->type = INFOS_LIST;
        content->infosList.nbInfos = 3;
        content->infosList.infoTypes = infoTypes;
        content->infosList.infoContents = infoContents;
    }

    return true;
}

static void ui_menu_about_wallet(void) {
  nbgl_useCaseSettings("Tezos wallet",
                       0, 1, false, ui_home_init, navigation_cb_wallet, NULL);
}

void tz_ui_home_redisplay(void) {
  FUNC_ENTER(("void"));
  nbgl_useCaseHome("Tezos", &C_tezos, NULL, false, ui_menu_about_wallet,
                   exit_app);
  FUNC_LEAVE();
}

#endif // HAVE_NBGL
