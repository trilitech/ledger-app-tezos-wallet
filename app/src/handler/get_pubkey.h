/* Tezos Ledger application - Handler for getting public key

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

#pragma once

#include <stdbool.h>

#include <buffer.h>

#include "keys.h"

/**
 * @brief Handle public key request.
 * If successfully parse BIP32 path, send APDU response containing the public
 * key.
 *
 * The public key is derived only once and stored in the RAM, in order to
 * avoid repeated derivation calculations.
 *
 * @param cdata: buffer containing the BIP32 path of the key
 * @param derivation_type: derivation_type of the key
 * @param prompt: whether to display address on screen or not
 */
void handle_get_public_key(buffer_t *cdata, derivation_type_t derivation_type,
                           bool prompt);
