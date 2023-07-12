/* Tezos Ledger application - Signature primitives

   TODO: cleanup/refactor

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

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "exception.h"
#include "memory.h"
#include "cx.h"

#define MAX_BIP32_LEN 10
#define SIGN_HASH_SIZE 32

typedef enum {
    DERIVATION_TYPE_SECP256K1 = 1,
    DERIVATION_TYPE_SECP256R1 = 2,
    DERIVATION_TYPE_ED25519 = 3,
    DERIVATION_TYPE_BIP32_ED25519 = 4
} derivation_type_t;

typedef struct {
    uint8_t length;
    uint32_t components[MAX_BIP32_LEN];
} bip32_path_t;

typedef struct {
    bip32_path_t bip32_path;
    derivation_type_t derivation_type;
} bip32_path_with_curve_t;

// throws
size_t read_bip32_path(bip32_path_t *, const uint8_t *, size_t);

int crypto_derive_private_key(cx_ecfp_private_key_t *, derivation_type_t,
                              const bip32_path_t *);

// Non-reentrant
void public_key_hash(uint8_t *, size_t, cx_ecfp_public_key_t *,
                     derivation_type_t, const cx_ecfp_public_key_t *);

size_t sign(uint8_t *, size_t, derivation_type_t, const cx_ecfp_private_key_t *,
            const uint8_t *, size_t);

static inline derivation_type_t parse_derivation_type(uint8_t curve_code) {
    switch (curve_code) {
        case 0:
            return DERIVATION_TYPE_ED25519;
        case 1:
            return DERIVATION_TYPE_SECP256K1;
        case 2:
            return DERIVATION_TYPE_SECP256R1;
        case 3:
            return DERIVATION_TYPE_BIP32_ED25519;
        default:
            THROW(EXC_WRONG_PARAM);
    }
}

int generate_public_key(cx_ecfp_public_key_t *, derivation_type_t,
                        const bip32_path_t *);
