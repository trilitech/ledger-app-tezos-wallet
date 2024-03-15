/* Tezos Embedded C parser for Ledger - Parser state for operations

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>

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

#include "num_state.h"

typedef enum {
    TZ_OPERATION_TAG_END          = 0,
    TZ_OPERATION_TAG_PROPOSALS    = 5,
    TZ_OPERATION_TAG_BALLOT       = 6,
    TZ_OPERATION_TAG_FAILING_NOOP = 17,
    TZ_OPERATION_TAG_REVEAL       = 107,
    TZ_OPERATION_TAG_TRANSACTION  = 108,
    TZ_OPERATION_TAG_ORIGINATION  = 109,
    TZ_OPERATION_TAG_DELEGATION   = 110,
    TZ_OPERATION_TAG_REG_GLB_CST  = 111,
    TZ_OPERATION_TAG_SET_DEPOSIT  = 112,
    TZ_OPERATION_TAG_INC_PAID_STG = 113,
    TZ_OPERATION_TAG_UPDATE_CK    = 114,
    TZ_OPERATION_TAG_TRANSFER_TCK = 158,
    TZ_OPERATION_TAG_SORU_ORIGIN  = 200,
    TZ_OPERATION_TAG_SORU_ADD_MSG = 201,
    TZ_OPERATION_TAG_SORU_EXE_MSG = 206
} tz_operation_tag;

typedef enum {
    TZ_OPERATION_STEP_OPTION,
    TZ_OPERATION_STEP_TUPLE,
    TZ_OPERATION_STEP_MAGIC,
    TZ_OPERATION_STEP_READ_BINARY,
    TZ_OPERATION_STEP_BRANCH,
    TZ_OPERATION_STEP_BATCH,
    TZ_OPERATION_STEP_TAG,
    TZ_OPERATION_STEP_SIZE,
    TZ_OPERATION_STEP_FIELD,
    TZ_OPERATION_STEP_PRINT,
    TZ_OPERATION_STEP_PARTIAL_PRINT,
    TZ_OPERATION_STEP_READ_NUM,
    TZ_OPERATION_STEP_READ_INT32,
    TZ_OPERATION_STEP_READ_PK,
    TZ_OPERATION_STEP_READ_BYTES,
    TZ_OPERATION_STEP_READ_STRING,
    TZ_OPERATION_STEP_READ_SMART_ENTRYPOINT,
    TZ_OPERATION_STEP_READ_MICHELINE,
    TZ_OPERATION_STEP_READ_SORU_MESSAGES,
    TZ_OPERATION_STEP_READ_SORU_KIND,
    TZ_OPERATION_STEP_READ_BALLOT,
    TZ_OPERATION_STEP_READ_PROTOS
} tz_operation_parser_step_kind;

typedef enum {
    TZ_OPERATION_FIELD_END = 0,  // not for use in field descriptors
    TZ_OPERATION_FIELD_OPTION,
    TZ_OPERATION_FIELD_TUPLE,
    TZ_OPERATION_FIELD_BINARY,
    TZ_OPERATION_FIELD_INT,
    TZ_OPERATION_FIELD_NAT,
    TZ_OPERATION_FIELD_AMOUNT,
    TZ_OPERATION_FIELD_FEE,
    TZ_OPERATION_FIELD_INT32,
    TZ_OPERATION_FIELD_STRING,
    TZ_OPERATION_FIELD_SOURCE,
    TZ_OPERATION_FIELD_PKH,
    TZ_OPERATION_FIELD_PK,
    TZ_OPERATION_FIELD_SR,
    TZ_OPERATION_FIELD_SRC,
    TZ_OPERATION_FIELD_PROTO,
    TZ_OPERATION_FIELD_PROTOS,
    TZ_OPERATION_FIELD_DESTINATION,
    TZ_OPERATION_FIELD_SMART_ENTRYPOINT,
    TZ_OPERATION_FIELD_EXPR,
    TZ_OPERATION_FIELD_OPH,
    TZ_OPERATION_FIELD_BH,
    TZ_OPERATION_FIELD_SORU_MESSAGES,
    TZ_OPERATION_FIELD_SORU_KIND,
    TZ_OPERATION_FIELD_BALLOT
} tz_operation_field_kind;

struct tz_operation_field_descriptor;

typedef struct {
    const struct tz_operation_field_descriptor *field;
    uint8_t                                     display_none : 1;
} tz_operation_option_field_descriptor;

typedef struct tz_operation_field_descriptor {
    const char             *name;
    tz_operation_field_kind kind : 5;
    union {
        tz_operation_option_field_descriptor field_option;
        struct {
            const struct tz_operation_field_descriptor *fields;
        } field_tuple;
    };
    uint8_t skip : 1, complex : 1;
} tz_operation_field_descriptor;

typedef struct {
    tz_operation_tag                     tag;
    const char                          *name;
    const tz_operation_field_descriptor *fields;
} tz_operation_descriptor;

typedef struct {
    tz_operation_parser_step_kind step : 5;
    uint16_t                      stop;
    union {
        tz_operation_option_field_descriptor step_option;
        struct {
            uint8_t  size_len;
            uint16_t size;
        } step_size;
        struct {
            const tz_operation_field_descriptor *field;
        } step_field;
        struct {
            const tz_operation_field_descriptor *fields;
            uint8_t                              field_index;
        } step_tuple;
        struct {
            const char *str;
        } step_print;
        struct {
            uint16_t                ofs;
            uint16_t                len;
            tz_operation_field_kind kind : 5;
            uint8_t                 skip : 1;
        } step_read_bytes;
        struct {
            tz_num_parser_regs      state;
            tz_operation_field_kind kind : 5;
            uint8_t                 skip : 1, natural : 1;
        } step_read_num;
        struct {
            int32_t value;
            uint8_t skip : 1, ofs : 3;
        } step_read_int32;
        struct {
            uint16_t ofs;
            uint8_t  skip : 1;
        } step_read_string;
        struct {
            const char *name;
            uint8_t     inited : 1;
            uint8_t     skip : 1;
        } step_read_micheline;
        struct {
            const char *name;
            uint16_t    index;
            uint8_t     skip : 1;
        } step_read_list;
    };
} tz_operation_parser_frame;

#define TZ_OPERATION_STACK_DEPTH 6

typedef struct {
    tz_operation_parser_frame  stack[TZ_OPERATION_STACK_DEPTH];
    tz_operation_parser_frame *frame;  // init == stack, NULL when done
    uint8_t                    seen_reveal : 1;  // check at most one reveal
    uint8_t  source[22];       // check consistent source in batch
    uint8_t  destination[22];  // saved for entrypoint dispatch
    uint16_t batch_index;      // to print a sequence number
#ifdef HAVE_SWAP
    tz_operation_tag last_tag;
    uint16_t         nb_reveal;
    uint64_t         last_fee;
    uint64_t         last_amount;
#endif  // HAVE_SWAP
} tz_operation_state;
