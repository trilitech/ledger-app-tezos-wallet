/* Tezos Embedded C parser for Ledger - Parser state for Micheline data

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

#pragma once

#include "num_state.h"

#define TZ_MICHELINE_STACK_DEPTH 45  /// Maximum micheline depth handled

/**
 * @brief Enumeration of all micheline tags
 */
typedef enum {
    TZ_MICHELINE_TAG_INT,
    TZ_MICHELINE_TAG_STRING,
    TZ_MICHELINE_TAG_SEQ,
    TZ_MICHELINE_TAG_PRIM_0_NOANNOTS,
    TZ_MICHELINE_TAG_PRIM_0_ANNOTS,
    TZ_MICHELINE_TAG_PRIM_1_NOANNOTS,
    TZ_MICHELINE_TAG_PRIM_1_ANNOTS,
    TZ_MICHELINE_TAG_PRIM_2_NOANNOTS,
    TZ_MICHELINE_TAG_PRIM_2_ANNOTS,
    TZ_MICHELINE_TAG_PRIM_N,
    TZ_MICHELINE_TAG_BYTES
} tz_micheline_tag;

/**
 * @brief Enumeration of all micheline parser step
 */
typedef enum {
    TZ_MICHELINE_STEP_TAG,
    TZ_MICHELINE_STEP_PRIM_OP,
    TZ_MICHELINE_STEP_PRIM_NAME,
    TZ_MICHELINE_STEP_PRIM,
    TZ_MICHELINE_STEP_SIZE,
    TZ_MICHELINE_STEP_SEQ,
    TZ_MICHELINE_STEP_BYTES,
    TZ_MICHELINE_STEP_STRING,
    TZ_MICHELINE_STEP_ANNOT,
    TZ_MICHELINE_STEP_INT,
    TZ_MICHELINE_STEP_PRINT_INT,
    TZ_MICHELINE_STEP_PRINT_CAPTURE
} tz_micheline_parser_step_kind;

/**
 * @brief
 */
typedef enum {
    TZ_CAP_STREAM_ANY    = 0,
    TZ_CAP_STREAM_BYTES  = 1,
    TZ_CAP_STREAM_INT    = 2,
    TZ_CAP_STREAM_STRING = 3,
    TZ_CAP_ADDRESS       = 4,
    TZ_CAP_LIST          = 62,
    TZ_CAP_OR            = 63
} tz_micheline_capture_kind;

/**
 * @brief This struct represents the frame of the parser of micheline
 *
 *        A frame contains the next step to be performed and its
 *        corresponding context
 */
typedef struct {
    tz_micheline_parser_step_kind step : 4;  /// step
    uint16_t                      stop;      /// stop offset
    union {
        struct {
            uint16_t size;  /// size read
        } step_size;        /// TZ_MICHELINE_STEP_SIZE
        struct {
            uint8_t first : 1;  /// if read first byte
        } step_seq;             /// TZ_MICHELINE_STEP_SEQ
        struct {
            uint8_t first : 1;         /// if read first byte
            uint8_t has_rem_half : 1;  /// if half the byte remains to print
            uint8_t rem_half;          /// remaining half of the byte
        } step_bytes;                  /// TZ_MICHELINE_STEP_BYTES
        struct {
            uint8_t first : 1;  /// if read first byte
        } step_string;          /// TZ_MICHELINE_STEP_STRING
        struct {
            uint8_t first : 1;        /// if read first byte
        } step_annot;                 /// TZ_MICHELINE_STEP_ANNOT
        tz_num_parser_regs step_int;  /// number parser register
                                      /// TZ_MICHELINE_STEP_INT,
                                      /// TZ_MICHELINE_STEP_PRINT_INT
        struct {
            uint8_t op;         /// prim op
            uint8_t ofs;        /// offset
            uint8_t nargs : 2;  /// number of arguments
            uint8_t wrap : 1;   /// if wrap in a prim
            uint8_t spc : 1;    /// if has space
            uint8_t annot : 1;  /// if need to read an annotation
            uint8_t first : 1;  /// if read first byte
        } step_prim;            /// TZ_MICHELINE_STEP_PRIM_OP,
                                /// TZ_MICHELINE_STEP_PRIM_NAME,
                                /// TZ_MICHELINE_STEP_PRIM
        struct {
            int ofs;     /// offset of the capture buffer
        } step_capture;  /// TZ_MICHELINE_STEP_CAPTURE_BYTES,
                         /// TZ_MICHELINE_STEP_PRINT_CAPTURE
    };
} tz_micheline_parser_frame;

/**
 * @brief This struct represents the parser of micheline
 *
 *        The parser is a one-by-one byte reader. It uses a stack
 *        automaton, for which each frame of the stack represents the
 *        reading states of the different layers of the micheline
 *        value read.
 */
typedef struct {
    tz_micheline_parser_frame
        stack[TZ_MICHELINE_STACK_DEPTH];  /// stack of frames
    tz_micheline_parser_frame *frame;     /// current frame
                                          /// init == stack, NULL when done
} tz_micheline_state;
