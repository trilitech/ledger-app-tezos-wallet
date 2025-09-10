/* Tezos Embedded C parser for Ledger - Human printing of Tezos formats

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include "formatting.h"

/**
 * @brief Ordered array of names for michelson primitives
 *
 * Should be kept in sync with the last protocol update, including
 * order, currently defined in the `michelson_v1_primitives.ml` file
 * in the Tezos protocol code.
 *
 * NEVER REORDER elements of this array as the index is used as a
 * selector by `michelson_op_name`.
 */
const char *const tz_michelson_op_names_ordered[TZ_LAST_MICHELSON_OPCODE + 1]
    = {
        "parameter",                       // 0
        "storage",                         // 1
        "code",                            // 2
        "False",                           // 3
        "Elt",                             // 4
        "Left",                            // 5
        "None",                            // 6
        "Pair",                            // 7
        "Right",                           // 8
        "Some",                            // 9
        "True",                            // 10
        "Unit",                            // 11
        "PACK",                            // 12
        "UNPACK",                          // 13
        "BLAKE2B",                         // 14
        "SHA256",                          // 15
        "SHA512",                          // 16
        "ABS",                             // 17
        "ADD",                             // 18
        "AMOUNT",                          // 19
        "AND",                             // 20
        "BALANCE",                         // 21
        "CAR",                             // 22
        "CDR",                             // 23
        "CHECK_SIGNATURE",                 // 24
        "COMPARE",                         // 25
        "CONCAT",                          // 26
        "CONS",                            // 27
        "CREATE_ACCOUNT",                  // 28
        "CREATE_CONTRACT",                 // 29
        "IMPLICIT_ACCOUNT",                // 30
        "DIP",                             // 31
        "DROP",                            // 32
        "DUP",                             // 33
        "EDIV",                            // 34
        "EMPTY_MAP",                       // 35
        "EMPTY_SET",                       // 36
        "EQ",                              // 37
        "EXEC",                            // 38
        "FAILWITH",                        // 39
        "GE",                              // 40
        "GET",                             // 41
        "GT",                              // 42
        "HASH_KEY",                        // 43
        "IF",                              // 44
        "IF_CONS",                         // 45
        "IF_LEFT",                         // 46
        "IF_NONE",                         // 47
        "INT",                             // 48
        "LAMBDA",                          // 49
        "LE",                              // 50
        "LEFT",                            // 51
        "LOOP",                            // 52
        "LSL",                             // 53
        "LSR",                             // 54
        "LT",                              // 55
        "MAP",                             // 56
        "MEM",                             // 57
        "MUL",                             // 58
        "NEG",                             // 59
        "NEQ",                             // 60
        "NIL",                             // 61
        "NONE",                            // 62
        "NOT",                             // 63
        "NOW",                             // 64
        "OR",                              // 65
        "PAIR",                            // 66
        "PUSH",                            // 67
        "RIGHT",                           // 68
        "SIZE",                            // 69
        "SOME",                            // 70
        "SOURCE",                          // 71
        "SENDER",                          // 72
        "SELF",                            // 73
        "STEPS_TO_QUOTA",                  // 74 [DEPRECATED]
        "SUB",                             // 75
        "SWAP",                            // 76
        "TRANSFER_TOKENS",                 // 77
        "SET_DELEGATE",                    // 78
        "UNIT",                            // 79
        "UPDATE",                          // 80
        "XOR",                             // 81
        "ITER",                            // 82
        "LOOP_LEFT",                       // 83
        "ADDRESS",                         // 84
        "CONTRACT",                        // 85
        "ISNAT",                           // 86
        "CAST",                            // 87
        "RENAME",                          // 88
        "bool",                            // 89
        "contract",                        // 90
        "int",                             // 91
        "key",                             // 92
        "key_hash",                        // 93
        "lambda",                          // 94
        "list",                            // 95
        "map",                             // 96
        "big_map",                         // 97
        "nat",                             // 98
        "option",                          // 99
        "or",                              // 100
        "pair",                            // 101
        "set",                             // 102
        "signature",                       // 103
        "string",                          // 104
        "bytes",                           // 105
        "mutez",                           // 106
        "timestamp",                       // 107
        "unit",                            // 108
        "operation",                       // 109
        "address",                         // 110
        "SLICE",                           // 111
        "DIG",                             // 112
        "DUG",                             // 113
        "EMPTY_BIG_MAP",                   // 114
        "APPLY",                           // 115
        "chain_id",                        // 116
        "CHAIN_ID",                        // 117
        "LEVEL",                           // 118
        "SELF_ADDRESS",                    // 119
        "never",                           // 120
        "NEVER",                           // 121
        "UNPAIR",                          // 122
        "VOTING_POWER",                    // 123
        "TOTAL_VOTING_POWER",              // 124
        "KECCAK",                          // 125
        "SHA3",                            // 126
        "PAIRING_CHECK",                   // 127
        "bls12_381_g1",                    // 128
        "bls12_381_g2",                    // 129
        "bls12_381_fr",                    // 130
        "sapling_state",                   // 131
        "sapling_transaction_deprecated",  // 132 [DEPRECATED]
        "SAPLING_EMPTY_STATE",             // 133
        "SAPLING_VERIFY_UPDATE",           // 134
        "ticket",                          // 135
        "TICKET_DEPRECATED",               // 136
        "READ_TICKET",                     // 137
        "SPLIT_TICKET",                    // 138
        "JOIN_TICKETS",                    // 139
        "GET_AND_UPDATE",                  // 140
        "chest",                           // 141
        "chest_key",                       // 142
        "OPEN_CHEST",                      // 143
        "VIEW",                            // 144
        "view",                            // 145
        "constant",                        // 146
        "SUB_MUTEZ",                       // 147
        "tx_rollup_l2_address",            // 148
        "MIN_BLOCK_TIME",                  // 149
        "sapling_transaction",             // 150
        "EMIT",                            // 151
        "Lambda_rec",                      // 152
        "LAMBDA_REC",                      // 153
        "TICKET",                          // 154
        "BYTES",                           // 155
        "NAT",                             // 156
        "Ticket",                          // 157
        "IS_IMPLICIT_ACCOUNT",             // 158
};

const char *
tz_michelson_op_name(uint8_t op_code)
{
    if (op_code > TZ_LAST_MICHELSON_OPCODE) {
        return NULL;
    }
    return PIC(tz_michelson_op_names_ordered[op_code]);
}

/*
 * The following `format_base58` and `format_decimal` are adaptaed
 * from `https://github.com/luke-jr/libbase58/blob/master/base58.c`,
 * mostly for working with less stack and a preallocated output
 * buffer. Copyright 2012-2014 Luke Dashjr
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the standard MIT license.
 */

static const char tz_b58digits_ordered[]
    = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

/**
 * @brief Get the base58 format of a number
 *
 * @param n: input number
 * @param l: length of the input buffer
 * @param obuf: output buffer
 * @param olen: length of the output buffer
 * @return int: 0 on success
 */
int
tz_format_base58(const uint8_t *n, size_t l, char *obuf, size_t olen)
{
    int    carry;
    size_t i, j, high, zcount = 0, obuf_len = TZ_BASE58_BUFFER_SIZE(l);

    if (olen < obuf_len) {
        PRINTF("[DEBUG] tz_format_base58() called with %u obuf need %u\n",
               olen, obuf_len);
        return 1;
    }

    memset(obuf, 0, obuf_len);

    while ((zcount < l) && !n[zcount]) {
        ++zcount;
    }

    for (i = zcount, high = obuf_len - 1; i < l; ++i, high = j) {
        carry = n[i];
        for (j = obuf_len - 1; ((int)j >= 0) && ((j > high) || carry); --j) {
            carry += 256 * obuf[j];
            obuf[j] = carry % 58;
            carry /= 58;
        }
    }

    if (zcount) {
        memset(obuf, '1', zcount);
    }

    for (j = 0; !obuf[j]; ++j) {
        // Find the last index of obuf
    }
    for (i = 0; j < obuf_len; ++i, ++j) {
        obuf[i] = tz_b58digits_ordered[(unsigned)obuf[j]];
    }
    obuf[i] = '\0';
    return 0;
}

int
tz_format_decimal(const uint8_t *n, size_t l, char *obuf, size_t olen)
{
    int    carry;
    size_t i, j, high, zcount = 0, obuf_len = TZ_DECIMAL_BUFFER_SIZE(l);

    if (olen < obuf_len) {
        PRINTF("[DEBUG] tz_format_base58() called with %u obuf need %u\n",
               olen, obuf_len);
        return 1;
    }

    memset(obuf, 0, obuf_len);

    while ((zcount < l) && !n[l - zcount - 1]) {
        ++zcount;
    }

    if (zcount == l) {
        obuf[0] = '0';
        return 0;
    }

    for (i = zcount, high = obuf_len - 1; i < l; ++i, high = j) {
        carry = n[l - i - 1];
        for (j = obuf_len - 1; ((int)j >= 0) && ((j > high) || carry); --j) {
            carry += 256 * obuf[j];
            obuf[j] = carry % 10;
            carry /= 10;
        }
    }

    for (j = 0; !obuf[j]; ++j) {
        // Find the last index of obuf
    }
    for (i = 0; j < obuf_len; ++i, ++j) {
        obuf[i] = '0' + obuf[j];
    }
    obuf[i] = '\0';
    return 0;
}

#ifndef ACTUALLY_ON_LEDGER
// Tezos links with digestif's C hashing functions, but the OPAM
// package does not publish their C header file for others to use, so
// we are forced to piggy import the external definitions in order to
// access them.

/**
 * @brief This struct represents the sha256 context
 */
struct sha256_ctx {
    uint64_t sz;
    uint8_t  buf[128];
    uint32_t h[8];
};

/**
 * @brief Initialize a sha256 context
 *
 * @param ctx: sha256 context
 */
extern void digestif_sha256_init(struct sha256_ctx *ctx);

/**
 * @brief Update the sha256 context with data
 *
 * @param ctx: sha256 context
 * @param data: data
 * @param size: length of the data
 */
extern void digestif_sha256_update(struct sha256_ctx *ctx, uint8_t *data,
                                   uint32_t size);

/**
 * @brief Finalize to digest the sha256 context
 *
 * @param ctx: sha256 context
 * @param out: output buffer
 */
extern void digestif_sha256_finalize(struct sha256_ctx *ctx, uint8_t *out);

/**
 * @brief Get the hash sha256 of a data
 *
 * @param data: data
 * @param size: length of the data
 * @param out: output buffer
 * @param size_out: length of the output buffer
 */
static void
cx_hash_sha256(uint8_t *data, size_t size, uint8_t *out, size_t size_out)
{
    struct sha256_ctx ctx;
    uint8_t           res[32];
    digestif_sha256_init(&ctx);
    digestif_sha256_update(&ctx, data, size);
    digestif_sha256_finalize(&ctx, res);
    memcpy(out, res, size_out);
}
#endif

// clang-format off
#define B58_PREFIX(_s, _p, _pl, _dl) do {       \
            if (!strcmp((_s), s)) {             \
                if ((_dl) != dl) {              \
                      return 1;                 \
                }                               \
                (*p)  = (const uint8_t *)(_p);  \
                (*pl) = (_pl);                  \
                return 0;                       \
            }                                   \
        } while (0)

/**
 * @brief Get base58 prefix information
 *
 *        To save lines and make things easier to read and modify, we
 *        implement find_prefix() as a series of invocations of the
 *        macro B58_PREFIX().  This macro takes 4 arguments: which
 *        correspond the the arguments of our function directly.  It
 *        matches the textual prefixes and if they do match, it sets
 *        the output parameters and validates that the length is
 *        correct.
 *
 *        find_prefix() returns successfully if it finds a definition
 *        and the length is correct.
 *
 * @param s: prefix we are looking for
 * @param p: an out parameter which is the binary prefix we matched
 * @param pl: another out parameter: the length of the returned binary prefix
 * @param dl: data length which we got
 * @return int: 0 on success
 */
static int
find_prefix(const char *s, const uint8_t **p, size_t *pl, size_t dl)
{

    /* For tz_format_hash */

    B58_PREFIX("B",     "\x01\x34",         2, 32);
    B58_PREFIX("o",     "\x05\x74",         2, 32);
    B58_PREFIX("expr",  "\x0d\x2c\x40\x1b", 4, 32);
    B58_PREFIX("proto", "\x02\xaa",         2, 32);

    /* Public key hashes */

    B58_PREFIX("tz1",  "\x06\xa1\x9f",     3, 20);
    B58_PREFIX("tz2",  "\x06\xa1\xa1",     3, 20);
    B58_PREFIX("tz3",  "\x06\xa1\xa4",     3, 20);
    B58_PREFIX("tz4",  "\x06\xa1\xa6",     3, 20);

    /* Public keys */

    B58_PREFIX("edpk", "\x0d\x0f\x25\xd9", 4, 32);
    B58_PREFIX("sppk", "\x03\xfe\xe2\x56", 4, 33);
    B58_PREFIX("p2pk", "\x03\xb2\x8b\x7f", 4, 33);
    B58_PREFIX("BLpk", "\x06\x95\x87\xcc", 4, 48);

    /* Signatures */

    B58_PREFIX("sig", "\x04\x82\x2b", 3, 64);
    B58_PREFIX("edsig", "\x09\xf5\xcd\x86\x12", 5, 64);
    B58_PREFIX("spsig1", "\x0d\x73\x65\x13\x3f", 5, 64);
    B58_PREFIX("p2sig", "\x36\xf0\x2c\x34", 4, 64);
    B58_PREFIX("BLsig", "\x28\xab\x40\xcf", 4, 96);

    /* For tz_format_address */

    B58_PREFIX("KT1",  "\x02\x5a\x79",     3, 20);
    B58_PREFIX("txr1", "\x01\x80\x78\x1f", 4, 20);
    B58_PREFIX("zkr1", "\x01\xab\x54\xfb", 4, 20);

    /* Smart rollup hashes */

    B58_PREFIX("sr1",  "\x06\x7c\x75",     3, 20);

    /* Smart rollup commitment hashes */

    B58_PREFIX("src1", "\x11\xa5\x86\x8a", 4, 32);

    return 1;
}
// clang-format on

int
tz_format_base58check(const char *sprefix, const uint8_t *data, size_t size,
                      char *obuf, size_t olen)
{
    const uint8_t *prefix = NULL;
    size_t         prefix_len;

    if (find_prefix(sprefix, &prefix, &prefix_len, size)) {
        return 1;
    }

    /* In order to avoid vla, we have a maximum buffer size of 128 */
    uint8_t prepared[128];
    if ((prefix_len + size + 4) > sizeof(prepared)) {
        PRINTF(
            "[WARNING] tz_format_base58check() failed: fixed size "
            "array is too small need: %u\n",
            prefix_len + size + 4);
        return 1;
    }

    memcpy(prepared, prefix, prefix_len);
    memcpy(prepared + prefix_len, data, size);
    uint8_t tmp[32];
    cx_hash_sha256(prepared, size + prefix_len, tmp, 32);
    cx_hash_sha256(tmp, 32, tmp, 32);
    memcpy(prepared + size + prefix_len, tmp, 4);
    return tz_format_base58(prepared, prefix_len + size + 4, obuf, olen);
}

int
tz_format_pkh(const uint8_t *data, size_t size, char *obuf, size_t olen)
{
    const char *prefix;

    if (size < 1) {
        return 1;
    }
    // clang-format off
    switch (data[0]) {
    case 0:  prefix = "tz1"; break;
    case 1:  prefix = "tz2"; break;
    case 2:  prefix = "tz3"; break;
    case 3:  prefix = "tz4"; break;
    default: return 1;
    }
    // clang-format on

    return tz_format_base58check(prefix, data + 1, size - 1, obuf, olen);
}

int
tz_format_pk(const uint8_t *data, size_t size, char *obuf, size_t olen)
{
    const char *prefix;

    if (size < 1) {
        return 1;
    }
    // clang-format off
    switch (data[0]) {
    case 0:  prefix = "edpk"; break;
    case 1:  prefix = "sppk"; break;
    case 2:  prefix = "p2pk"; break;
    case 3:  prefix = "BLpk"; break;
    default: return 1;
    }
    // clang-format on

    return tz_format_base58check(prefix, data + 1, size - 1, obuf, olen);
}

int
tz_format_sig(const uint8_t *data, size_t size, char *obuf, size_t olen)
{
    const char *prefix;

    // clang-format off
    switch (size) {
    case 64:  prefix = "sig"; break;
    case 96:  prefix = "BLsig"; break;
    default: return 1;
    }
    // clang-format on

    return tz_format_base58check(prefix, data, size, obuf, olen);
}

int
tz_format_oph(const uint8_t *data, size_t size, char *obuf, size_t olen)
{
    return tz_format_base58check("o", data, size, obuf, olen);
}

int
tz_format_bh(const uint8_t *data, size_t size, char *obuf, size_t olen)
{
    return tz_format_base58check("B", data, size, obuf, olen);
}

int
tz_format_address(const uint8_t *data, size_t size, char *obuf, size_t olen)
{
    const char *prefix;

    if (size < 1) {
        return 1;
    }
    // clang-format off
    switch (data[0]) {
    case 1:  prefix = "KT1";  break;
    case 2:  prefix = "txr1"; break;
    case 3:  prefix = "scr1"; break;
    case 4:  prefix = "zkr1"; break;

    case 0:  return tz_format_pkh(data+1, size-1, obuf, olen);
    default: return 1;
    }
    // clang-format on

    return tz_format_base58check(prefix, data + 1, size - 2, obuf, olen);
}
