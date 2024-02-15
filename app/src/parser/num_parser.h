/* Tezos Embedded C parser for Ledger - Big num parser

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

#include "parser_state.h"

/**
 * @brief Initialize a num state buffer and a num state regs
 *
 * @param buffers: number parser buffers
 * @param regs: number parser register
 */
void tz_parse_num_state_init(tz_num_parser_buffer *buffers,
                             tz_num_parser_regs   *regs);

/**
 * @brief Parse one byte
 *
 * @param buffers: number parser buffers
 * @param regs: number parser register
 * @param b: byte to parse
 * @param natural: if the number to read is natural
 * @return tz_parser_result: parser result
 */
tz_parser_result tz_parse_num_step(tz_num_parser_buffer *buffers,
                                   tz_num_parser_regs *regs, uint8_t b,
                                   bool natural);

/**
 * @brief Parse one byte to read an integer
 *
 * @param buffers: number parser buffers
 * @param regs: number parser register
 * @param b: byte to parse
 * @return tz_parser_result: parser result
 */
tz_parser_result tz_parse_int_step(tz_num_parser_buffer *buffers,
                                   tz_num_parser_regs *regs, uint8_t b);

/**
 * @brief Parse one byte to read a natural number
 *
 * @param buffers: number parser buffers
 * @param regs: number parser register
 * @param b: byte to parse
 * @return tz_parser_result: parser result
 */
tz_parser_result tz_parse_nat_step(tz_num_parser_buffer *buffers,
                                   tz_num_parser_regs *regs, uint8_t b);

/**
 * @brief format a buffer to mutez number
 *
 * @param in: intput buffer
 * @param out: output number
 * @return bool: success
 */
bool tz_string_to_mutez(const char *in, uint64_t *out);
