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

#pragma once

#include "parser_state.h"

/**
 * @brief Initialize a micheline parser state
 *
 * @param state: parser state
 */
void tz_micheline_parser_init(tz_parser_state *state);

/**
 * @brief Apply one step to the micheline parser
 *
 * @param state: parser state
 * @return tz_parser_result: parser result
 */
tz_parser_result tz_micheline_parser_step(tz_parser_state *state);
