/* Tezos Ledger application - Public key review display

   Copyright 2025 Functori <contact@functori.com>
   Copyright 2024 TriliTech <contact@trili.tech>
   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

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

#include "parser/formatting.h"
#include "keys.h"
#include "ui_commons.h"

#define PUBKEY_HASH_SIZE TZ_BASE58CHECK_BUFFER_SIZE(20, 3)

/**
 * @brief Holds data for public key review.
 */
typedef struct {
    char               address[PUBKEY_HASH_SIZE];
    action_validate_cb callback;
} tz_ui_pubkey_t;

/**
 * @brief Displays a public key review request
 *
 * @param pubkey: Public key
 * @param derivation_type: Public key derivation type
 * @param callback: Action to process on confirm/reject
 */
void ui_pubkey_review(cx_ecfp_public_key_t *pubkey,
                      derivation_type_t     derivation_type,
                      action_validate_cb    callback);
