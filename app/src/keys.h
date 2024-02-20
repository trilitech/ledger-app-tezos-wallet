/* Tezos Ledger application - Signature primitives

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

#include <memory.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <cx.h>

#include "exception.h"

#define MAX_BIP32_LEN  10
#define SIGN_HASH_SIZE 32

/**
 * @brief The derivation type values in the following enum are from the
 * on-the-wire protocol
 *
 */
typedef enum {
    DERIVATION_TYPE_ED25519       = 0,
    DERIVATION_TYPE_SECP256K1     = 1,
    DERIVATION_TYPE_SECP256R1     = 2,
    DERIVATION_TYPE_BIP32_ED25519 = 3,
    DERIVATION_TYPE_MAX           = 4
} derivation_type_t;

typedef struct {
    uint8_t  length;
    uint32_t components[MAX_BIP32_LEN];
} bip32_path_t;

typedef struct {
    bip32_path_t      bip32_path;
    derivation_type_t derivation_type;
} bip32_path_with_curve_t;

tz_exc read_bip32_path(bip32_path_t *out, const uint8_t *in, size_t in_size);

/**
 * @brief Derive public key for given derivation type address.
 *
 * @param public_key Public key derived is stored in this struct.
 * @param derivation_type Derivation type to be used
 * @param bip32_path path to derive public key from
 * @return tz_exc return success/failure using error code
 */
tz_exc derive_pk(cx_ecfp_public_key_t *public_key,
                 derivation_type_t     derivation_type,
                 const bip32_path_t   *bip32_path);
/**
 * @brief Derive hash of public key for given derivation type address.
 *
 * @param hash The hash of public key, output is stored in this buffer.
 * @param derivation_type Derivation type to be used.
 * @param bip32_path Path to derive public key from.
 * @return tz_exc return Error code
 */
tz_exc derive_pkh(cx_ecfp_public_key_t *pubkey,
                  derivation_type_t derivation_type, char *buffer,
                  size_t len);
void   sign(derivation_type_t derivation_type, const bip32_path_t *path,
            const uint8_t *hash, size_t hashlen, uint8_t *sig, size_t *siglen);

/**
 * @brief Check if derivation type is valid enum
 *
 * @param code Derivation type to check.
 * @return validity result
 */
static inline bool
check_derivation_type(derivation_type_t code)
{
    return ((code >= DERIVATION_TYPE_ED25519)
            && (code < DERIVATION_TYPE_MAX));
}
