/* Tezos Ledger application - Tezos-specific formatting function set

   Copyright 2024 Functori <contact@functori.com>

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

#include <format.h>

#include "format.h"

bool
tz_mutez_to_string(char *obuf, size_t olen, uint64_t amount)
{
    if (!format_fpu64_trimmed(obuf, olen, amount, 6 /*DECIMALS*/)) {
        memset(obuf, '\0', olen);
        return false;
    }

    strlcat(obuf, " XTZ", olen);
    return true;
}
