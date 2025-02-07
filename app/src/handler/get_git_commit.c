/* Tezos Ledger application - Handler for getting git commit

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

#include <io.h>

#include "get_git_commit.h"

#include "exception.h"
#include "utils.h"

void
handle_get_git_commit(void)
{
    FUNC_ENTER(("void"));

    static const char commit[] = COMMIT;
    io_send_response_pointer((void *)commit, sizeof(commit), SW_OK);

    FUNC_LEAVE();
}
