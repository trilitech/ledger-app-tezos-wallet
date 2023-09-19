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

typedef struct {
    uint16_t size;
    uint8_t sign : 1, stop : 1;
} tz_num_parser_regs;

#define TZ_NUM_BUFFER_SIZE 256

typedef struct {
    uint8_t bytes[TZ_NUM_BUFFER_SIZE/8];
    char decimal[TZ_DECIMAL_BUFFER_SIZE(TZ_NUM_BUFFER_SIZE/8)];
} tz_num_parser_buffer;

