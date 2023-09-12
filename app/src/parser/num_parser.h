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

void tz_parse_num_state_init(tz_num_parser_buffer *, tz_num_parser_regs *);
tz_parser_result tz_parse_num_step(tz_num_parser_buffer *,
                                   tz_num_parser_regs *, uint8_t, bool);
tz_parser_result tz_parse_int_step(tz_num_parser_buffer *,
                                   tz_num_parser_regs *, uint8_t);
tz_parser_result tz_parse_nat_step(tz_num_parser_buffer *,
                                   tz_num_parser_regs *, uint8_t);

bool tz_string_to_mutez(const char *, uint64_t *);
