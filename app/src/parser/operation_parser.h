/* Tezos Embedded C parser for Ledger - Operation parser

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

#include "parser_state.h"

#define TZ_UNKNOWN_SIZE 0xFFFFu

/**
 * @brief Initialize a operations parser state
 *
 *        As operations can be very large, the size of the data to
 *        parse can be unknown in that case size can be set to
 *        `TZ_UNKNOWN_SIZE`. The size can then be set with
 *        `tz_operation_parser_set_size`
 *
 * @param state: parser state
 * @param size: size of operations
 * @param skip_magic: set it to false if a magic byte needs to be
 *                    analysed to find out whether the bytes represent
 *                    a micheline expression or a batch of
 *                    operations. Otherwise, it will assume that the
 *                    bytes represent a batch of operations.
 */
void tz_operation_parser_init(tz_parser_state *state, uint16_t size,
                              bool skip_magic);

/**
 * @brief Set the operations size
 *
 * @param state: parser state
 * @param size: size of operations
 */
void tz_operation_parser_set_size(tz_parser_state *state, uint16_t size);

/**
 * @brief Apply one step to the operations parser
 *
 * @param state: parser state
 * @return tz_parser_result: parser result
 */
tz_parser_result tz_operation_parser_step(tz_parser_state *state);
