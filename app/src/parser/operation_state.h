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

/**
 * @brief Enumeration of all operations tags
 */
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

/**
 * @brief Enumeration of all operations parser step
 */
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
    TZ_OPERATION_STEP_READ_PROTOS,
    TZ_OPERATION_STEP_READ_PKH_LIST
} tz_operation_parser_step_kind;

/**
 * @brief Enumeration of all operations fields
 */
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
    TZ_OPERATION_FIELD_PKH_LIST,
    TZ_OPERATION_FIELD_BALLOT
} tz_operation_field_kind;

struct tz_operation_field_descriptor;

/**
 * @brief This struct represents the field descriptor of an option field
 */
typedef struct {
    const struct tz_operation_field_descriptor *field;  /// field descriptor
    uint8_t display_none : 1;                           /// display if is none
} tz_operation_option_field_descriptor;

/**
 * @brief This struct represents the descriptor of field
 */
typedef struct tz_operation_field_descriptor {
    const char             *name;      /// name
    tz_operation_field_kind kind : 5;  /// kind
    union {
        tz_operation_option_field_descriptor
            field_option;  /// field of the option
                           ///    TZ_OPERATION_FIELD_OPTION

        struct {
            const struct tz_operation_field_descriptor
                *fields;  /// fields of the tuple
        } field_tuple;    /// TZ_OPERATION_FIELD_TUPLE
    };
    uint8_t skip : 1;     /// if the field is not printed
    uint8_t complex : 1;  /// if the field is considered too complex for a
                          /// common user
} tz_operation_field_descriptor;

/**
 * @brief This struct represents the descriptor of operations
 */
typedef struct {
    tz_operation_tag                     tag;     /// tag
    const char                          *name;    /// name
    const tz_operation_field_descriptor *fields;  /// fields
} tz_operation_descriptor;

/**
 * @brief This struct represents the frame of the parser of operations
 *
 *        A frame contains the next step to be performed and its
 *        corresponding context
 */
typedef struct {
    tz_operation_parser_step_kind step : 5;  /// step
    uint16_t                      stop;      /// stop offset
    union {
        tz_operation_option_field_descriptor
            step_option;  /// option field
                          /// TZ_OPERATION_STEP_OPTION
        struct {
            uint8_t  size_len;  /// number of bytes to read
            uint16_t size;      /// current parsed value
        } step_size;            /// TZ_OPERATION_STEP_SIZE
        struct {
            const tz_operation_field_descriptor
                *field;  /// field that need to be read
        } step_field;    /// TZ_OPERATION_STEP_FIELD
        struct {
            const tz_operation_field_descriptor
                   *fields;       /// fields of the tuple
            uint8_t field_index;  /// index of the current field to read
        } step_tuple;             /// TZ_OPERATION_STEP_TUPLE
        struct {
            const char *str;  /// string to print
        } step_print;         /// TZ_OPERATION_STEP_PRINT
                              /// TZ_OPERATION_STEP_PARTIAL_PRINT
        struct {
            uint16_t ofs;  /// current bytes buffer offset
            uint16_t len;  /// expected bytes length
            tz_operation_field_kind
                kind : 5;      /// kind of field
                               /// TZ_OPERATION_FIELD_SOURCE
                               /// TZ_OPERATION_FIELD_PKH
                               /// TZ_OPERATION_FIELD_PK
                               /// TZ_OPERATION_FIELD_SR
                               /// TZ_OPERATION_FIELD_SRC
                               /// TZ_OPERATION_FIELD_PROTO
                               /// TZ_OPERATION_FIELD_DESTINATION
                               /// TZ_OPERATION_FIELD_OPH
                               /// TZ_OPERATION_FIELD_BH
            uint8_t skip : 1;  /// if the field is skipped
        } step_read_bytes;     /// TZ_OPERATION_STEP_READ_BYTES
        struct {
            tz_num_parser_regs      state;     /// number parser register
            tz_operation_field_kind kind : 5;  /// what kind of field
                                               /// TZ_OPERATION_FIELD_NAT
                                               /// TZ_OPERATION_FIELD_FEE
                                               /// TZ_OPERATION_FIELD_AMOUNT
                                               /// TZ_OPERATION_FIELD_INT
            uint8_t skip : 1;                  /// if the field is skipped
            uint8_t natural : 1;               /// if its a natural number
        } step_read_num;                       /// TZ_OPERATION_STEP_READ_NUM
        struct {
            int32_t value;     /// current value parsed
            uint8_t skip : 1;  /// if the field is skipped
            uint8_t ofs : 3;   /// number offset
        } step_read_int32;     /// TZ_OPERATION_STEP_READ_INT32
        struct {
            uint16_t ofs;       /// current buffer string offset
            uint8_t  skip : 1;  /// if the field is skipped
        } step_read_string;     /// TZ_OPERATION_STEP_READ_STRING
                                /// TZ_OPERATION_STEP_READ_BINARY
        struct {
            const char *name;  /// field name
            uint8_t
                inited : 1;    /// if the parser micheline has been initialize
            uint8_t skip : 1;  /// if the field is skipped
        } step_read_micheline;  /// TZ_OPERATION_STEP_READ_MICHELINE
        struct {
            const char *name;      /// field name
            uint16_t    index;     /// current index in the list
            uint8_t     skip : 1;  /// if the field is skipped
        } step_read_list;          /// TZ_OPERATION_STEP_READ_PROTOS
                                   /// TZ_OPERATION_STEP_READ_SORU_MESSAGES
    };
} tz_operation_parser_frame;

#define TZ_OPERATION_STACK_DEPTH 6  /// Maximum operations depth handled

/**
 * @brief This struct represents the parser of operations
 *
 *        The parser is a one-by-one byte reader. It uses a stack
 *        automaton, for which each frame of the stack represents the
 *        reading states of the different layers of the operations
 *        value read.
 */
typedef struct {
    tz_operation_parser_frame
        stack[TZ_OPERATION_STACK_DEPTH];  /// stack of frames
    tz_operation_parser_frame *frame;     /// current frame
                                          /// init == stack, NULL when done
    uint8_t  seen_reveal : 1;             /// check at most one reveal
    uint8_t  source[22];                  /// check consistent source in batch
    uint8_t  destination[22];             /// saved for entrypoint dispatch
    uint16_t batch_index;                 /// to print a sequence number
#ifdef HAVE_SWAP
    tz_operation_tag last_tag;   /// last operations tag encountered
    uint16_t         nb_reveal;  /// number of reveal encountered
#endif                           // HAVE_SWAP
    uint64_t total_fee;          /// last fee encountered
    uint64_t total_amount;       /// last amount encountered
} tz_operation_state;
