/* Tezos Embedded C parser for Ledger - Micheline data parser

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

#ifndef _TZ_MICHELINE_PARSER_H
#define _TZ_MICHELINE_PARSER_H	1

#include "parser_state.h"

extern void tz_micheline_parser_init(tz_parser_state *state, const uint8_t *pat, size_t pat_len);
extern tz_parser_result tz_micheline_parser_step(tz_parser_state *state, tz_parser_regs *regs);

#endif /* micheline_parser.h */
