/* Tezos Ledger application - Signature primitives

   TODO: cleanup/refactor

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

#include "buffer.h"
#include "crypto_helpers.h"

#include "keys.h"

#include "apdu.h"
#include "globals.h"

static cx_curve_t derivation_type_to_cx_curve(derivation_type_t const
                                              derivation_type) {
  switch (derivation_type) {
  case DERIVATION_TYPE_ED25519:
  case DERIVATION_TYPE_BIP32_ED25519:
    return CX_CURVE_Ed25519;
  case DERIVATION_TYPE_SECP256K1:
    return CX_CURVE_SECP256K1;
  case DERIVATION_TYPE_SECP256R1:
    return CX_CURVE_SECP256R1;
  default:
    return CX_CURVE_NONE;
  }
}

cx_err_t read_bip32_path(bip32_path_t *const out, uint8_t const *const in,
                         size_t in_size) {
    buffer_t cdata = {in, in_size, 0};
    cx_err_t ret = CX_OK;

    FUNC_ENTER(("out=%p, in=%p, in_size=%u", out, in, in_size));

    if(!buffer_read_u8(&cdata, &out->length) ||
       !buffer_read_bip32_path(&cdata, (uint32_t *) &out->components,
                               out->length)) {
      ret = EXC_WRONG_LENGTH_FOR_INS;
    }

    FUNC_LEAVE();
    return ret;
}

cx_err_t generate_public_key(cx_ecfp_public_key_t *public_key,
                             derivation_type_t const derivation_type,
                             bip32_path_t const *const bip32_path) {
    cx_err_t error = CX_OK;

    FUNC_ENTER(("public_key=%p, derivation_type=%d, bip32_path=%p",
                public_key, derivation_type, bip32_path));

    public_key->W_len = 65;
    public_key->curve = derivation_type_to_cx_curve(derivation_type);

    int derivation_mode = HDW_NORMAL;
    if (derivation_type == DERIVATION_TYPE_ED25519)
      derivation_mode = HDW_ED25519_SLIP10;

    CX_CHECK(bip32_derive_with_seed_get_pubkey_256(derivation_mode,
                                                   public_key->curve,
                                                   bip32_path->components,
                                                   bip32_path->length,
                                                   public_key->W,
                                                   NULL,
                                                   CX_SHA512,
                                                   NULL, 0));

    if (public_key->curve == CX_CURVE_Ed25519) {
        CX_CHECK(cx_edwards_compress_point_no_throw(CX_CURVE_Ed25519,
                                                    public_key->W,
                                                    public_key->W_len));
        public_key->W_len = 33;
    }

end:
    FUNC_LEAVE();
    return error;
}

#define HASH_SIZE 20

cx_err_t public_key_hash(uint8_t *hash_out, size_t hash_out_size,
                         cx_ecfp_public_key_t *compressed_out,
                         derivation_type_t derivation_type,
                         const cx_ecfp_public_key_t *public_key) {
    cx_err_t error = CX_OK;

    FUNC_ENTER(("hash_out=%p, hash_out_size=%u, compressed_out=%p, "
                "derivation_type=%d, public_key=%p",
                hash_out, hash_out_size, compressed_out,
                derivation_type, public_key));
    check_null(hash_out);
    check_null(public_key);
    if (hash_out_size < HASH_SIZE) 
        return EXC_WRONG_LENGTH;

    cx_ecfp_public_key_t compressed = {0};
    switch (derivation_type) {
        case DERIVATION_TYPE_BIP32_ED25519:
        case DERIVATION_TYPE_ED25519: {
            compressed.W_len = public_key->W_len - 1;
            memcpy(compressed.W, public_key->W + 1, compressed.W_len);
            break;
        }
        case DERIVATION_TYPE_SECP256K1:
        case DERIVATION_TYPE_SECP256R1: {
            memcpy(compressed.W, public_key->W, public_key->W_len);
            compressed.W[0] = 0x02 + (public_key->W[64] & 0x01);
            compressed.W_len = 33;
            break;
        }
        default:
            return EXC_WRONG_PARAM;
    }

    cx_blake2b_t hash_state;
    // cx_blake2b_init_no_throw takes size in bits.
    CX_CHECK(cx_blake2b_init_no_throw(&hash_state, HASH_SIZE * 8));
    CX_CHECK(cx_hash_no_throw((cx_hash_t *) &hash_state,
			      CX_LAST,
			      compressed.W,
			      compressed.W_len,
			      hash_out,
			      HASH_SIZE));
    if (compressed_out != NULL) {
        memmove(compressed_out, &compressed, sizeof(*compressed_out));
    }

end:
    FUNC_LEAVE();
    return error;
}

cx_err_t sign(derivation_type_t derivation_type,
              const bip32_path_t *path,
              const uint8_t *hash, size_t hashlen,
              uint8_t *sig, size_t *siglen) {
    unsigned derivation_mode;
    uint32_t info;
    cx_curve_t curve = derivation_type_to_cx_curve(derivation_type);
    cx_err_t err = EXC_WRONG_PARAM;

    check_null(hash);
    check_null(path);
    check_null(sig);
    check_null(siglen);
    FUNC_ENTER(("sig=%p, siglen=%u, derivation_type=%d, "
                "path=%p, hash=%p, hashlen=%u",
                sig, *siglen, derivation_type, path, hash, hashlen));

    switch (derivation_type) {
    case DERIVATION_TYPE_BIP32_ED25519:
    case DERIVATION_TYPE_ED25519:
        derivation_mode = HDW_NORMAL;
        if (derivation_type == DERIVATION_TYPE_ED25519)
            derivation_mode = HDW_ED25519_SLIP10;
        err = bip32_derive_with_seed_eddsa_sign_hash_256(derivation_mode,
                                                         curve,
                                                         path->components,
                                                         path->length,
                                                         CX_SHA512,
                                                         hash, hashlen,
                                                         sig, siglen,
                                                         NULL, 0);
        break;
    case DERIVATION_TYPE_SECP256K1:
    case DERIVATION_TYPE_SECP256R1:
        err = bip32_derive_ecdsa_sign_hash_256(curve,
                                               path->components, path->length,
                                               CX_RND_RFC6979 | CX_LAST,
                                               CX_SHA256,
                                               hash, hashlen,
                                               sig, siglen, &info);
        if (info & CX_ECCINFO_PARITY_ODD)
            sig[0] |= 0x01;
        break;
    default:
        break;
    }

    FUNC_LEAVE();
    return err;
}
