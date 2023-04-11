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

#pragma once

#include "os.h"

// Throw this to indicate prompting
#define ASYNC_EXCEPTION 0x2000

// Standard APDU error codes:
// https://www.eftlab.co.uk/index.php/site-map/knowledge-base/118-apdu-response-list

#define EXC_WRONG_PARAM               0x6B00
#define EXC_WRONG_LENGTH              0x6C00
#define EXC_INVALID_INS               0x6D00
#define EXC_WRONG_LENGTH_FOR_INS      0x917E
#define EXC_REJECT                    0x6985
#define EXC_PARSE_ERROR               0x9405
#define EXC_REFERENCED_DATA_NOT_FOUND 0x6A88
#define EXC_WRONG_VALUES              0x6A80
#define EXC_SECURITY                  0x6982
#define EXC_HID_REQUIRED              0x6983
#define EXC_CLASS                     0x6E00
#define EXC_MEMORY_ERROR              0x9200

#define EXC_UNEXPECTED_STATE          0x9001
#define EXC_UNEXPECTED_SIGN_STATE     0x9002
#define EXC_UNKNOWN                   0x90FF

// Crashes can be harder to debug than exceptions and latency isn't a big concern
static inline void check_null(void volatile const *const ptr) {
    if (ptr == NULL) {
        THROW(EXC_MEMORY_ERROR);
    }
}

#ifdef TEZOS_DEBUG
static inline __attribute((noreturn)) void failwith(const char* message) {
  PRINTF("[ERROR] %s\n", message);
  THROW (EXC_UNEXPECTED_STATE);
}
#else
static inline __attribute((noreturn)) void failwith( __attribute((unused)) const char* message) {
  THROW (EXC_UNEXPECTED_STATE);
}
#endif
