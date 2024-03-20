/* Tezos Ledger application - Signature primitives

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>

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

#include <stdbool.h>
#include <string.h>

#include <buffer.h>
#include <crypto_helpers.h>
#include <io.h>

#include "apdu.h"
#include "exception.h"
#include "keys.h"
#include "globals.h"

static tz_exc public_key_hash(uint8_t *hash_out, size_t hash_out_size,
                              cx_ecfp_public_key_t       *compressed_out,
                              derivation_type_t           derivation_type,
                              const cx_ecfp_public_key_t *public_key);

static cx_curve_t
derivation_type_to_cx_curve(derivation_type_t derivation_type)
{
    // clang-format off
    switch (derivation_type) {
    case DERIVATION_TYPE_ED25519:
    case DERIVATION_TYPE_BIP32_ED25519: return CX_CURVE_Ed25519;
    case DERIVATION_TYPE_SECP256K1:     return CX_CURVE_SECP256K1;
    case DERIVATION_TYPE_SECP256R1:     return CX_CURVE_SECP256R1;
    default:                            return CX_CURVE_NONE;
    }
    // clang-format on
}

tz_exc
read_bip32_path(bip32_path_t *out, const uint8_t *in, size_t in_size)
{
    buffer_t cdata = {in, in_size, 0};
    TZ_PREAMBLE(("out=%p, in=%p, in_size=%u", out, in, in_size));

    TZ_ASSERT(EXC_WRONG_LENGTH_FOR_INS,
              buffer_read_u8(&cdata, &out->length)
                  && buffer_read_bip32_path(
                      &cdata, (uint32_t *)&out->components, out->length)
                  // Assert entire bip32_path consumed
                  && (sizeof(uint8_t) + sizeof(uint32_t) * out->length
                      == cdata.offset));
    TZ_LIB_POSTAMBLE;
}

tz_exc
derive_pk(cx_ecfp_public_key_t *public_key, derivation_type_t derivation_type,
          const bip32_path_t *bip32_path)
{
    TZ_PREAMBLE(("public_key=%p, derivation_type=%d, bip32_path=%p",
                 public_key, derivation_type, bip32_path));

    public_key->W_len = 65;
    public_key->curve = derivation_type_to_cx_curve(derivation_type);

    int derivation_mode = HDW_NORMAL;
    if (derivation_type == DERIVATION_TYPE_ED25519) {
        derivation_mode = HDW_ED25519_SLIP10;
    }

    CX_CHECK(bip32_derive_with_seed_get_pubkey_256(
        derivation_mode, public_key->curve, bip32_path->components,
        bip32_path->length, public_key->W, NULL, CX_SHA512, NULL, 0));

    if (public_key->curve == CX_CURVE_Ed25519) {
        CX_CHECK(cx_edwards_compress_point_no_throw(
            CX_CURVE_Ed25519, public_key->W, public_key->W_len));
        public_key->W_len = 33;
    }

    TZ_LIB_POSTAMBLE;
}

tz_exc
derive_pkh(cx_ecfp_public_key_t *pubkey, derivation_type_t derivation_type,
           char *buffer, size_t len)
{
    uint8_t hash[21];
    TZ_PREAMBLE(("buffer=%p, len=%u", buffer, len));
    TZ_ASSERT_NOTNULL(buffer);
    TZ_LIB_CHECK(
        public_key_hash(hash + 1, 20, NULL, derivation_type, pubkey));
    // clang-format off
    switch (derivation_type) {
    case DERIVATION_TYPE_SECP256K1: hash[0] = 1; break;
    case DERIVATION_TYPE_SECP256R1: hash[0] = 2; break;
    case DERIVATION_TYPE_ED25519:
    case DERIVATION_TYPE_BIP32_ED25519: hash[0] = 0; break;
    default: TZ_FAIL(EXC_WRONG_PARAM); break;
    }
    // clang-format on

    if (tz_format_pkh(hash, 21, buffer, len)) {
        TZ_FAIL(EXC_UNKNOWN);
    }

    TZ_LIB_POSTAMBLE;
}

#define HASH_SIZE 20

static tz_exc
public_key_hash(uint8_t *hash_out, size_t hash_out_size,
                cx_ecfp_public_key_t       *compressed_out,
                derivation_type_t           derivation_type,
                const cx_ecfp_public_key_t *public_key)
{
    TZ_PREAMBLE(
        ("hash_out=%p, hash_out_size=%u, compressed_out=%p, "
         "derivation_type=%d, public_key=%p",
         hash_out, hash_out_size, compressed_out, derivation_type,
         public_key));

    TZ_ASSERT_NOTNULL(hash_out);
    TZ_ASSERT_NOTNULL(public_key);
    TZ_ASSERT(EXC_WRONG_LENGTH, hash_out_size >= HASH_SIZE);

    cx_ecfp_public_key_t compressed = {0};
    switch (derivation_type) {
    case DERIVATION_TYPE_BIP32_ED25519:
    case DERIVATION_TYPE_ED25519:
        compressed.W_len = public_key->W_len - 1;
        memcpy(compressed.W, public_key->W + 1, compressed.W_len);
        break;
    case DERIVATION_TYPE_SECP256K1:
    case DERIVATION_TYPE_SECP256R1:
        memcpy(compressed.W, public_key->W, public_key->W_len);
        compressed.W[0]  = 0x02 + (public_key->W[64] & 0x01);
        compressed.W_len = 33;
        break;
    default:
        TZ_FAIL(EXC_WRONG_PARAM);
    }

    cx_blake2b_t hash_state;
    CX_CHECK(cx_blake2b_init_no_throw(&hash_state, HASH_SIZE * 8));
    CX_CHECK(cx_hash_no_throw((cx_hash_t *)&hash_state, CX_LAST, compressed.W,
                              compressed.W_len, hash_out, HASH_SIZE));
    if (compressed_out != NULL) {
        memmove(compressed_out, &compressed, sizeof(*compressed_out));
    }

    TZ_LIB_POSTAMBLE;
}

/**
 * @brief   Sign a hash with eddsa using the device seed derived from the
 * specified bip32 path and seed key.
 *
 * @param[in]  derivation_type Derivation type, ex. ED25519
 *
 * @param[in]  path            Bip32 path to use for derivation.
 *
 * @param[in]  hash            Digest of the message to be signed.
 *
 * @param[in]  hash_len        Length of the digest in octets.
 *
 * @param[out] sig             Buffer where to store the signature.
 *
 * @param[in]  sig_len         Length of the signature buffer, updated with
 * signature length.
 */
void
sign(derivation_type_t derivation_type, const bip32_path_t *path,
     const uint8_t *hash, size_t hashlen, uint8_t *sig, size_t *siglen)
{
    unsigned   derivation_mode;
    uint32_t   info;
    cx_curve_t curve = derivation_type_to_cx_curve(derivation_type);
    TZ_PREAMBLE(
        ("sig=%p, siglen=%u, derivation_type=%d, "
         "path=%p, hash=%p, hashlen=%u",
         sig, *siglen, derivation_type, path, hash, hashlen));
    TZ_ASSERT_NOTNULL(path);
    TZ_ASSERT_NOTNULL(hash);
    TZ_ASSERT_NOTNULL(sig);
    TZ_ASSERT_NOTNULL(siglen);

    switch (derivation_type) {
    case DERIVATION_TYPE_BIP32_ED25519:
    case DERIVATION_TYPE_ED25519:
        derivation_mode = HDW_NORMAL;
        if (derivation_type == DERIVATION_TYPE_ED25519) {
            derivation_mode = HDW_ED25519_SLIP10;
        }
        CX_CHECK(bip32_derive_with_seed_eddsa_sign_hash_256(
            derivation_mode, curve, path->components, path->length, CX_SHA512,
            hash, hashlen, sig, siglen, NULL, 0));
        break;
    case DERIVATION_TYPE_SECP256K1:
    case DERIVATION_TYPE_SECP256R1:
        CX_CHECK(bip32_derive_ecdsa_sign_hash_256(
            curve, path->components, path->length, CX_RND_RFC6979 | CX_LAST,
            CX_SHA256, hash, hashlen, sig, siglen, &info));
        if (info & CX_ECCINFO_PARITY_ODD) {
            sig[0] |= 0x01;
        }
        break;
    default:
        TZ_FAIL(EXC_WRONG_VALUES);
        break;
    }
    TZ_POSTAMBLE;
}
