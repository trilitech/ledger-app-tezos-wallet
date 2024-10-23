/* Tezos Ledger application - Global application state

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

#include <os.h>
#include <ux.h>

#include "globals.h"

globals_t global;

const settings_t N_settings_real;

void
init_globals(void)
{
    memset(&global, 0, sizeof(global));
    memset(G_io_seproxyhal_spi_buffer, 0, sizeof(G_io_seproxyhal_spi_buffer));
    memset(&G_ux, 0, sizeof(G_ux));
    memset(&G_ux_params, 0, sizeof(G_ux_params));
}

void
toggle_expert_mode(void)
{
    settings_t tmp;
    memcpy(&tmp, (void *)&N_settings, sizeof(tmp));
    tmp.expert_mode = !N_settings.expert_mode;
    nvm_write((void *)&N_settings, (void *)&tmp, sizeof(N_settings));
}

void
toggle_blindsigning(void)
{
    settings_t tmp;
    memcpy(&tmp, (void *)&N_settings, sizeof(tmp));
    tmp.blindsigning = !N_settings.blindsigning;
    nvm_write((void *)&N_settings, (void *)&tmp, sizeof(N_settings));
}
