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

#include "keys.h"

#include "apdu.h"
#include "globals.h"

static cx_curve_t derivation_type_to_cx_curve(derivation_type_t const derivation_type) {
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

#define READ_UNALIGNED_BE(type, in) ({                  \
        uint8_t const *_bytes = (uint8_t const *)in;    \
        type _res;                                      \
                                                        \
        for (size_t _i = 0; _i < sizeof(type); _i++)    \
            _res = ((_res) << 8) | _bytes[_i];          \
                                                        \
        _res;                                           \
    })

#define CONSUME_UNALIGNED_BE(counter, type, addr) ({    \
        counter += sizeof(type);                        \
        READ_UNALIGNED_BE(type, addr);                  \
    })

struct bip32_path_wire {
    uint8_t length;
    uint32_t components[0];
} __attribute__((packed));

size_t read_bip32_path(bip32_path_t *const out, uint8_t const *const in,
                       size_t in_size) {
    struct bip32_path_wire const *const buf_as_bip32 = (struct bip32_path_wire const *) in;

    FUNC_ENTER(("out=%p, in=%p, in_size=%u", out, in, in_size));

    if (in_size < sizeof(buf_as_bip32->length)) THROW(EXC_WRONG_LENGTH_FOR_INS);

    size_t ix = 0;
    out->length = CONSUME_UNALIGNED_BE(ix, uint8_t, &buf_as_bip32->length);

    if (in_size - ix < out->length * sizeof(*buf_as_bip32->components))
        THROW(EXC_WRONG_LENGTH_FOR_INS);
    if (out->length == 0 || out->length > MAX_BIP32_LEN) THROW(EXC_WRONG_VALUES);

    for (size_t i = 0; i < out->length; i++) {
        out->components[i] =
            CONSUME_UNALIGNED_BE(ix, uint32_t, &buf_as_bip32->components[i]);
    }

    FUNC_LEAVE();
    return ix;
}


int crypto_derive_private_key(cx_ecfp_private_key_t *private_key,
                              derivation_type_t const derivation_type,
                              bip32_path_t const *const bip32_path) {
    check_null(bip32_path);
    uint8_t raw_private_key[PRIVATE_KEY_DATA_SIZE] = {0};
    cx_err_t err;

    FUNC_ENTER(("private_key=%p, derivation_type=%d, bip32_path=%p",
                private_key, derivation_type, bip32_path));

    cx_curve_t const cx_curve =
        derivation_type_to_cx_curve(derivation_type);

    if (derivation_type == DERIVATION_TYPE_ED25519) {
	// Old, non BIP32_Ed25519 way...
	err = os_derive_bip32_with_seed_no_throw(HDW_ED25519_SLIP10,
						 CX_CURVE_Ed25519,
						 bip32_path->components,
						 bip32_path->length,
						 raw_private_key,
						 NULL, NULL, 0);
    } else {
	// derive the seed with bip32_path
	err = os_derive_bip32_no_throw(cx_curve, bip32_path->components,
				       bip32_path->length, raw_private_key,
				       NULL);
    }

    if (!err)
        err = cx_ecfp_init_private_key_no_throw(cx_curve, raw_private_key,
                                                32, private_key);

    explicit_bzero(raw_private_key, sizeof(raw_private_key));

    FUNC_LEAVE();
    return err ? 1 : 0;
}

int crypto_init_public_key(derivation_type_t const derivation_type,
                           cx_ecfp_private_key_t *private_key,
                           cx_ecfp_public_key_t *public_key) {
    FUNC_ENTER(("derivation_type=%d, private_key=%p, public_key=%p",
                derivation_type, private_key, public_key));
    cx_curve_t const cx_curve =
        derivation_type_to_cx_curve(derivation_type);

    // generate corresponding public key
    CX_THROW(cx_ecfp_generate_pair_no_throw(cx_curve, public_key, private_key, 1));

    // If we're using the old curve, make sure to adjust accordingly.
    if (cx_curve == CX_CURVE_Ed25519) {
         cx_edwards_compress_point_no_throw(CX_CURVE_Ed25519, public_key->W, public_key->W_len);
        public_key->W_len = 33;
    }

    FUNC_LEAVE();
    return 0;
}

int generate_public_key(cx_ecfp_public_key_t *public_key,
                        derivation_type_t const derivation_type,
                        bip32_path_t const *const bip32_path) {
    cx_ecfp_private_key_t private_key = {0};
    int error;

    FUNC_ENTER(("public_key=%p, derivation_type=%d, bip32_path=%p",
                public_key, derivation_type, bip32_path));

    error = crypto_derive_private_key(&private_key, derivation_type, bip32_path);
    if (error) {
        return error;
    }
    error = crypto_init_public_key(derivation_type, &private_key, public_key);
    FUNC_LEAVE();
    return error;
}

#define HASH_SIZE 20

void public_key_hash(uint8_t *const hash_out,
                     size_t const hash_out_size,
                     cx_ecfp_public_key_t *compressed_out,
                     derivation_type_t const derivation_type,
                     cx_ecfp_public_key_t const *const public_key) {
    FUNC_ENTER(("hash_out=%p, hash_out_size=%u, compressed_out=%p, "
                "derivation_type=%d, public_key=%p",
                hash_out, hash_out_size, compressed_out,
                derivation_type, public_key));
    check_null(hash_out);
    check_null(public_key);
    if (hash_out_size < HASH_SIZE) THROW(EXC_WRONG_LENGTH);

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
            THROW(EXC_WRONG_PARAM);
    }

    cx_blake2b_t hash_state;
    CX_THROW(cx_blake2b_init_no_throw(&hash_state, HASH_SIZE * 8));  // cx_blake2b_init_no_throw takes size in bits.
    CX_THROW(cx_hash_no_throw((cx_hash_t *) &hash_state,
			      CX_LAST,
			      compressed.W,
			      compressed.W_len,
			      hash_out,
			      HASH_SIZE));
    if (compressed_out != NULL) {
        memmove(compressed_out, &compressed, sizeof(*compressed_out));
    }
    FUNC_LEAVE();
}

size_t sign(uint8_t *const out,
            size_t const out_size,
            derivation_type_t const derivation_type,
            cx_ecfp_private_key_t const *priv,
            uint8_t const *const in,
            size_t const in_size) {
    FUNC_ENTER(("out=%p, out_size=%u, derivation_type=%d, "
                "pair=%p, in=%p, in_size=%u",
                out, out_size, derivation_type, priv, in, in_size));
    check_null(out);
    check_null(priv);
    check_null(in);

    size_t tx = 0;
    switch (derivation_type) {
        case DERIVATION_TYPE_BIP32_ED25519:
        case DERIVATION_TYPE_ED25519: {
            static size_t const SIG_SIZE = 64;
            if (out_size < SIG_SIZE) THROW(EXC_WRONG_LENGTH);
            CX_THROW(cx_eddsa_sign_no_throw(priv,
					    CX_SHA512,
					    (uint8_t const *) PIC(in),
					    in_size,
					    out,
					    SIG_SIZE));
	    tx += SIG_SIZE;
        } break;
        case DERIVATION_TYPE_SECP256K1:
        case DERIVATION_TYPE_SECP256R1: {
            static size_t const SIG_SIZE = 100;
            if (out_size < SIG_SIZE) THROW(EXC_WRONG_LENGTH);
            uint32_t info;
	    size_t sig_len = SIG_SIZE;
            CX_THROW(cx_ecdsa_sign_no_throw(priv,
					    CX_LAST | CX_RND_RFC6979,
					    CX_SHA256,  // historical reasons...semantically CX_NONE
					    (uint8_t const *) PIC(in),
					    in_size,
					    out,
					    &sig_len,
					    &info));
	    tx += sig_len;
            if (info & CX_ECCINFO_PARITY_ODD) {
                out[0] |= 0x01;
            }
        } break;
        default:
            THROW(EXC_WRONG_PARAM);  // This should not be able to happen.
    }

    FUNC_LEAVE();
    return tx;
}
