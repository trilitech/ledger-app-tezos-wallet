/* Tezos Embedded C parser for Ledger - Big num parser

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

#ifndef _TZ_NUM_PARSER_H
#define _TZ_NUM_PARSER_H	1

#include "parser_state.h"

extern void tz_parse_num_state_init (tz_num_parser_buffer *buffers,
                                     tz_num_parser_regs *regs);
extern tz_parser_result tz_parse_num_step (tz_num_parser_buffer *buffers,
                                           tz_num_parser_regs *regs,
                                           uint8_t b, bool natural, bool pretty_print);
extern tz_parser_result tz_parse_int_step (tz_num_parser_buffer *buffers,
                                           tz_num_parser_regs *regs,
                                           uint8_t b, bool pretty_print);
extern tz_parser_result tz_parse_nat_step (tz_num_parser_buffer *buffers,
                                           tz_num_parser_regs *regs,
                                           uint8_t b, bool pretty_print);

#endif /* num_state.h */
