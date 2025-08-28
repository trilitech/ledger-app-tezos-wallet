/* Tezos Ledger application - Instruction dispatcher

   Copyright 2025 Functori <contact@functori.com>
   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

   With code excerpts from:
    - Legacy Tezos app, Copyright 2019 Obsidian Systems
    - Ledger Blue sample apps, Copyright 2016 Ledger

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

#include <parser.h>

/**
 * @brief Read the APDU for the order and choose the next action to take.
 *
 * The APDU structure is defined by [ISO/IEC
 * 7816-4](https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit).
 * This function ensures that the command complies with the [APDU
 * specification for our application](app/docs/apdu.md) and, depending on the
 * code instruction, parse the instruction parameters in order to supply them,
 * in addition to the potential command data, to the corresponding process.
 *
 * @param cmd: command containg APDU received
 */
void dispatch(const command_t *cmd);
