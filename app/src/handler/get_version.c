/* Tezos Ledger application - Handler for getting app version

   Copyright 2025 Functori <contact@functori.com>
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

#include <stdint.h>

#include <io.h>

#include "get_version.h"

#include "exception.h"
#include "utils.h"

const uint8_t version[4]
    = {0 /* wallet */, MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION};

void
handle_get_version(void)
{
    FUNC_ENTER(("void"));

    io_send_response_pointer((void *)&version, sizeof(version), SW_OK);

    FUNC_LEAVE();
}
