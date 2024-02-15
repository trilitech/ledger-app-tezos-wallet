/* Tezos Embedded C parser for Ledger - Parser state for big nums

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

#include "formatting.h"

/**
 * @brief This struct represents the register for the parser of a number
 *
 *        The parser is a one-by-one byte reader.
 */
typedef struct {
    uint16_t size;      /// size of the number
    uint8_t  sign : 1;  /// sign ot the number
    uint8_t  stop : 1;  /// number as been fully parsed
} tz_num_parser_regs;

#define TZ_NUM_BUFFER_SIZE 256  /// Size of the number buffer

/**
 * @brief This struct represents the output buffers for the parser of a number
 */
typedef struct {
    uint8_t bytes[TZ_NUM_BUFFER_SIZE / 8];                         /// bytes
    char decimal[TZ_DECIMAL_BUFFER_SIZE(TZ_NUM_BUFFER_SIZE / 8)];  /// decimal
} tz_num_parser_buffer;
