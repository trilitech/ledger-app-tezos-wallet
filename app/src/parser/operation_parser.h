/* Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#ifndef _TZ_OPERATION_PARSER_H
#define _TZ_OPERATION_PARSER_H	1

#include "parser_state.h"

#define TZ_UNKNOWN_SIZE 0xFFFF
extern void tz_operation_parser_init(tz_parser_state *state, uint16_t size, bool skip_magic);
extern void tz_operation_parser_set_size(tz_parser_state *state, uint16_t size);
extern tz_parser_result tz_operation_parser_step(tz_parser_state *state, tz_parser_regs *regs);

#endif /* operation_parser.h */
