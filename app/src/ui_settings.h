/* Tezos Ledger application - Initialize Nano settings

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
#pragma once

#define SETTINGS_HOME_PAGE      0
#define SETTINGS_BLINDSIGN_PAGE 1

/**
 * @brief Initialize settings screen for nano devices. Displays status of
 * Expert-mode and Blind Signing.
 *
 * @param page Current page to display among all the pages available under
 * Settings.
 */
void ui_settings_init(int16_t page);
#endif  // HAVE_BAGL
