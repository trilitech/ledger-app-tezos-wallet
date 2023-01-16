#include "formatting.h"

// Should be kept in sync with the last protocol update, including
// order, currently defined in the `michelson_v1_primitives.ml` file
// in the Tezos protocol code.
//
// NEVER REORDER elements of this array as the index is used as a
// selector by `michelson_op_name`.
const char* const tz_michelson_op_names_ordered[TZ_LAST_MICHELSON_OPCODE+1] = {
  "parameter",                      // 0
  "storage",                        // 1
  "code",                           // 2
  "False",                          // 3
  "Elt",                            // 4
  "Left",                           // 5
  "None",                           // 6
  "Pair",                           // 7
  "Right",                          // 8
  "Some",                           // 9
  "True",                           // 10
  "Unit",                           // 11
  "PACK",                           // 12
  "UNPACK",                         // 13
  "BLAKE2B",                        // 14
  "SHA256",                         // 15
  "SHA512",                         // 16
  "ABS",                            // 17
  "ADD",                            // 18
  "AMOUNT",                         // 19
  "AND",                            // 20
  "BALANCE",                        // 21
  "CAR",                            // 22
  "CDR",                            // 23
  "CHECK_SIGNATURE",                // 24
  "COMPARE",                        // 25
  "CONCAT",                         // 26
  "CONS",                           // 27
  "CREATE_ACCOUNT",                 // 28
  "CREATE_CONTRACT",                // 29
  "IMPLICIT_ACCOUNT",               // 30
  "DIP",                            // 31
  "DROP",                           // 32
  "DUP",                            // 33
  "EDIV",                           // 34
  "EMPTY_MAP",                      // 35
  "EMPTY_SET",                      // 36
  "EQ",                             // 37
  "EXEC",                           // 38
  "FAILWITH",                       // 39
  "GE",                             // 40
  "GET",                            // 41
  "GT",                             // 42
  "HASH_KEY",                       // 43
  "IF",                             // 44
  "IF_CONS",                        // 45
  "IF_LEFT",                        // 46
  "IF_NONE",                        // 47
  "INT",                            // 48
  "LAMBDA",                         // 49
  "LE",                             // 50
  "LEFT",                           // 51
  "LOOP",                           // 52
  "LSL",                            // 53
  "LSR",                            // 54
  "LT",                             // 55
  "MAP",                            // 56
  "MEM",                            // 57
  "MUL",                            // 58
  "NEG",                            // 59
  "NEQ",                            // 60
  "NIL",                            // 61
  "NONE",                           // 62
  "NOT",                            // 63
  "NOW",                            // 64
  "OR",                             // 65
  "PAIR",                           // 66
  "PUSH",                           // 67
  "RIGHT",                          // 68
  "SIZE",                           // 69
  "SOME",                           // 70
  "SOURCE",                         // 71
  "SENDER",                         // 72
  "SELF",                           // 73
  "STEPS_TO_QUOTA",                 // 74 [DEPRECATED]
  "SUB",                            // 75
  "SWAP",                           // 76
  "TRANSFER_TOKENS",                // 77
  "SET_DELEGATE",                   // 78
  "UNIT",                           // 79
  "UPDATE",                         // 80
  "XOR",                            // 81
  "ITER",                           // 82
  "LOOP_LEFT",                      // 83
  "ADDRESS",                        // 84
  "CONTRACT",                       // 85
  "ISNAT",                          // 86
  "CAST",                           // 87
  "RENAME",                         // 88
  "bool",                           // 89
  "contract",                       // 90
  "int",                            // 91
  "key",                            // 92
  "key_hash",                       // 93
  "lambda",                         // 94
  "list",                           // 95
  "map",                            // 96
  "big_map",                        // 97
  "nat",                            // 98
  "option",                         // 99
  "or",                             // 100
  "pair",                           // 101
  "set",                            // 102
  "signature",                      // 103
  "string",                         // 104
  "bytes",                          // 105
  "mutez",                          // 106
  "timestamp",                      // 107
  "unit",                           // 108
  "operation",                      // 109
  "address",                        // 110
  "SLICE",                          // 111
  "DIG",                            // 112
  "DUG",                            // 113
  "EMPTY_BIG_MAP",                  // 114
  "APPLY",                          // 115
  "chain_id",                       // 116
  "CHAIN_ID",                       // 117
  "LEVEL",                          // 118
  "SELF_ADDRESS",                   // 119
  "never",                          // 120
  "NEVER",                          // 121
  "UNPAIR",                         // 122
  "VOTING_POWER",                   // 123
  "TOTAL_VOTING_POWER",             // 124
  "KECCAK",                         // 125
  "SHA3",                           // 126
  "PAIRING_CHECK",                  // 127
  "bls12_381_g1",                   // 128
  "bls12_381_g2",                   // 129
  "bls12_381_fr",                   // 130
  "sapling_state",                  // 131
  "sapling_transaction_deprecated", // 132 [DEPRECATED]
  "SAPLING_EMPTY_STATE",            // 133
  "SAPLING_VERIFY_UPDATE",          // 134
  "ticket",                         // 135
  "TICKET_DEPRECATED",              // 136
  "READ_TICKET",                    // 137
  "SPLIT_TICKET",                   // 138
  "JOIN_TICKETS",                   // 139
  "GET_AND_UPDATE",                 // 140
  "chest",                          // 141
  "chest_key",                      // 142
  "OPEN_CHEST",                     // 143
  "VIEW",                           // 144
  "view",                           // 145
  "constant",                       // 146
  "SUB_MUTEZ",                      // 147
  "tx_rollup_l2_address",           // 148
  "MIN_BLOCK_TIME",                 // 149
  "sapling_transaction",            // 150
  "EMIT",                           // 151
  "Lambda_rec",                     // 152
  "LAMBDA_REC",                     // 153
  "TICKET"                          // 154
};

const char* tz_michelson_op_name (uint8_t op_code) {
  if (op_code > TZ_LAST_MICHELSON_OPCODE) return NULL;
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

static const char tz_b58digits_ordered[] =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

void tz_format_base58(const uint8_t *n, size_t l, char *obuf) {
  int carry;
  size_t i, j, high, zcount = 0, obuf_len = TZ_BASE58_BUFFER_SIZE(l);

  memset(obuf, 0, obuf_len);

  while (zcount < l && !n[zcount]) ++zcount;

  for (i = zcount, high = obuf_len - 1; i < l; ++i, high = j) {
    for (carry = n[i], j = obuf_len - 1; ((int) j >= 0) && ((j > high) || carry); --j) {
      carry += 256 * obuf[j];
      obuf[j] = carry % 58;
      carry /= 58;
    }
  }

  if (zcount) memset(obuf, '1', zcount);

  for (j = 0; !obuf[j]; ++j);
  for (i = 0; j < obuf_len; ++i, ++j) obuf[i] = tz_b58digits_ordered[(unsigned)obuf[j]];
  obuf[i] = '\0';
}

void tz_format_decimal (const uint8_t *n, size_t l, char *obuf) {
  int carry;
  size_t i, j, high, zcount = 0, obuf_len = TZ_DECIMAL_BUFFER_SIZE(l);

  memset(obuf, 0, obuf_len);

  while (zcount < l && !n[l-zcount-1]) ++zcount;

  if (zcount == l) {
    obuf[0] = '0';
    return;
  }

  for (i = zcount, high = obuf_len - 1; i < l; ++i, high = j) {
    for (carry = n[l-i-1], j = obuf_len - 1; ((int) j >= 0) && ((j > high) || carry); --j) {
      carry += 256 * obuf[j];
      obuf[j] = carry % 10;
      carry /= 10;
    }
  }

  for (j = 0; !obuf[j]; ++j);
  for (i = 0; j < obuf_len; ++i, ++j) obuf[i] = '0' + obuf[j];
  obuf[i] = '\0';
}

#ifndef ACTUALLY_ON_LEDGER
// Tezos links with digestif's C hashing functions, but the OPAM
// package does not publish their C header file for others to use, so
// we are forced to piggy import the external definitions in order to
// access them.
struct sha256_ctx { uint64_t sz; uint8_t  buf[128]; uint32_t h[8];};
extern void digestif_sha256_init(struct sha256_ctx *ctx);
extern void digestif_sha256_update(struct sha256_ctx *ctx, uint8_t *data, uint32_t len);
extern void digestif_sha256_finalize(struct sha256_ctx *ctx, uint8_t *out);
static void cx_hash_sha256(uint8_t *data, size_t size, uint8_t *out, size_t size_out) {
  struct sha256_ctx ctx;
  uint8_t res[32];
  digestif_sha256_init(&ctx);
  digestif_sha256_update(&ctx, data, size);
  digestif_sha256_finalize(&ctx, res);
  memcpy(out, res, size_out);
}
#endif

void tz_format_base58check (const uint8_t *prefix, size_t prefix_len, const uint8_t *data, size_t size, char *obuf) {
  uint8_t prepared[prefix_len+size+4];
  memcpy(prepared, prefix, prefix_len);
  memcpy(prepared+prefix_len, data, size);
  uint8_t tmp[32];
  cx_hash_sha256(prepared, size+prefix_len, tmp, 32);
  cx_hash_sha256(tmp, 32, tmp, 32);
  memcpy(prepared+size+prefix_len, tmp, 4);
  tz_format_base58(prepared, prefix_len+size+4, obuf);
}

int tz_format_pkh(const uint8_t *data, size_t size, char *obuf) {
  if (size != 21) return 1;
  switch (data[0]) {
  case 0: tz_format_base58check((uint8_t*)"\x06\xa1\x9f", 3, data+1, 20, obuf); break; // tz1
  case 1: tz_format_base58check((uint8_t*)"\x06\xa1\xa1", 3, data+1, 20, obuf); break; // tz2
  case 2: tz_format_base58check((uint8_t*)"\x06\xa1\xa4", 3, data+1, 20, obuf); break; // tz3
  case 3: tz_format_base58check((uint8_t*)"\x06\xa1\xa6", 3, data+1, 20, obuf); break; // tz4
  default: return 1;
  }
  return 0;
}

int tz_format_pk(const uint8_t *data, size_t size, char *obuf) {
  if (size < 1) return 1;
  switch (data[0]) {
  case 0:
    if (size != 33) return 1;
    tz_format_base58check((uint8_t*)"\x0d\x0f\x25\xd9", 4, data+1, 32, obuf); break; // edpk
  case 1:
    if (size != 34) return 1;
    tz_format_base58check((uint8_t*)"\x03\xfe\xe2\x56", 4, data+1, 33, obuf); break; // sppk
  case 2:
    if (size != 34) return 1;
    tz_format_base58check((uint8_t*)"\x03\xb2\x8b\x7f", 4, data+1, 33, obuf); break; // p2pk
  case 3:
    if (size != 49) return 1;
    tz_format_base58check((uint8_t*)"\x06\x95\x87\xcc", 4, data+1, 48, obuf); break; // BLpk
  default: return 1;
  }
  return 0;
}

int tz_format_oph(const uint8_t *data, size_t size, char *obuf) {
  if (size != 32) return 1;
  tz_format_base58check((uint8_t*)"\x05\x74", 2, data, 32, obuf); // op
  return 0;
}

int tz_format_bh(const uint8_t *data, size_t size, char *obuf) {
  if (size != 32) return 1;
  tz_format_base58check((uint8_t*)"\x01\x34", 2, data, 32, obuf); // Bl
  return 0;
}

int tz_format_address(const uint8_t *data, size_t size, char *obuf) {
  if (size != 22) return 1;
  switch (data[0]) {
  case 0:
    return tz_format_pkh (data+1, 21, obuf);
  case 1:
    tz_format_base58check((uint8_t*)"\x02\x5a\x79", 3, data+1, 20, obuf); break; // KT1
  case 2:
    tz_format_base58check((uint8_t*)"\x01\x80\x78\x1f", 4, data+1, 20, obuf); break; // txr1
  case 3:
    tz_format_base58check((uint8_t*)"\x01\x76\x84\xd9", 4, data+1, 20, obuf); break; // scr1
  case 4:
    tz_format_base58check((uint8_t*)"\x01\xab\x54\xfb", 4, data+1, 20, obuf); break; // zkr1
  default: return 1;
  }
  return 0;
}
