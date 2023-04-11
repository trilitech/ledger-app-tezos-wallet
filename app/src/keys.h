/* Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

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
    cx_ecfp_public_key_t public_key;
    cx_ecfp_private_key_t private_key;
} key_pair_t;

typedef struct {
    uint8_t length;
    uint32_t components[MAX_BIP32_LEN];
} bip32_path_t;

typedef struct {
    bip32_path_t bip32_path;
    derivation_type_t derivation_type;
} bip32_path_with_curve_t;

// throws
size_t read_bip32_path(bip32_path_t *const out, uint8_t const *const in, size_t const in_size);

int generate_key_pair(key_pair_t *key_pair,
                      derivation_type_t const derivation_type,
                      bip32_path_t const *const bip32_path);

// Non-reentrant
void public_key_hash(
    uint8_t *const hash_out,
    size_t const hash_out_size,
    cx_ecfp_public_key_t *const compressed_out,  // pass NULL if this value is not desired
    derivation_type_t const derivation_type,
    cx_ecfp_public_key_t const *const restrict public_key);

size_t sign(uint8_t *const out,
            size_t const out_size,
            derivation_type_t const derivation_type,
            key_pair_t const *const key,
            uint8_t const *const in,
            size_t const in_size);

static inline derivation_type_t parse_derivation_type(uint8_t const curve_code) {
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

int generate_public_key(cx_ecfp_public_key_t *public_key,
                        derivation_type_t const derivation_type,
                        bip32_path_t const *const bip32_path);
