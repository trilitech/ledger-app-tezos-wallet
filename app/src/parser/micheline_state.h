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

#define TZ_MICHELINE_STACK_DEPTH 45

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

typedef enum {
    TZ_CAP_STREAM_ANY    = 0,
    TZ_CAP_STREAM_BYTES  = 1,
    TZ_CAP_STREAM_INT    = 2,
    TZ_CAP_STREAM_STRING = 3,
    TZ_CAP_ADDRESS       = 4,
    TZ_CAP_LIST          = 62,
    TZ_CAP_OR            = 63
} tz_micheline_capture_kind;

typedef struct {
    tz_micheline_parser_step_kind step : 4;
    uint16_t                      stop;
    union {
        struct {
            uint16_t size;
        } step_size;  // TZ_MICHELINE_STEP_SIZE
        struct {
            uint8_t first : 1;
        } step_seq;  // TZ_MICHELINE_STEP_SEQ
        struct {
            uint8_t first : 1, has_rem_half : 1;
            uint8_t rem_half;
        } step_bytes;  // TZ_MICHELINE_STEP_BYTES
        struct {
            uint8_t first : 1;
        } step_string;  // TZ_MICHELINE_STEP_STRING
        struct {
            uint8_t first : 1;
        } step_annot;                 // TZ_MICHELINE_STEP_ANNOT
        tz_num_parser_regs step_int;  // TZ_MICHELINE_STEP_INT,
                                      // TZ_MICHELINE_STEP_PRINT_INT
        struct {
            uint8_t op;
            uint8_t ofs;
            uint8_t nargs : 2, wrap : 1, spc : 1, annot : 1, first : 1;
        } step_prim;  // TZ_MICHELINE_STEP_PRIM_OP,
                      // TZ_MICHELINE_STEP_PRIM_NAME,
                      // TZ_MICHELINE_STEP_PRIM
        struct {
            int ofs;
        } step_capture;  // TZ_MICHELINE_STEP_CAPTURE_BYTES,
                         // TZ_MICHELINE_STEP_PRINT_CAPTURE
        tz_micheline_capture_kind
            step_capturing;  // TZ_MICHELINE_STEP_CAPTURING
    };
} tz_micheline_parser_frame;

typedef struct {
    tz_micheline_parser_frame  stack[TZ_MICHELINE_STACK_DEPTH];
    tz_micheline_parser_frame *frame;  // init == stack, NULL when done
} tz_micheline_state;
