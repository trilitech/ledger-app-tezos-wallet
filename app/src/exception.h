/* Tezos Ledger application - Error handling primitives

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

#include <os.h>

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
#define EXC_SWAP_CHECKING_FAIL        0x6A8E
#define EXC_REFERENCED_DATA_NOT_FOUND 0x6A88
#define EXC_WRONG_VALUES              0x6A80
#define EXC_SECURITY                  0x6982
#define EXC_HID_REQUIRED              0x6983
#define EXC_CLASS                     0x6E00
#define EXC_MEMORY_ERROR              0x9200

#define EXC_UNEXPECTED_STATE          0x9001
#define EXC_UNEXPECTED_SIGN_STATE     0x9002
#define EXC_UNKNOWN                   0x90FF

#define TZ_ASSERT(_err, _cond) do {                                         \
                if (!(_cond)) {                                             \
                    PRINTF("Assertion (\"%s\") on %s:%u failed with %s\n",  \
                           #_cond, __FILE__, __LINE__, #_err);              \
                    error = (_err);                                         \
                    goto end;                                               \
                }                                                           \
            } while (0)

/*
 * TZ_CHECK() has the same calling conventions as CX_CHECK including using
 * the same variable (error) and label (end).  This is by design as we
 * intend to use it in the same way, but we do not want to imply that it
 * is only for crypto functions.
 *
 * We expect "tz_err_t error" to be defined in the environment, and a label
 * "end:" at the end of the function which may do some cleanup and then will
 * return "error".
 */

typedef uint32_t tz_err_t;
#define TZ_OK 0x0000

#define TZ_CHECK(_call) do {                                                \
                error = (_call);                                            \
                if (error) {                                                \
                    PRINTF("TZ_CHECK(\"%s\") on %s:%u failed with %u\n",    \
                           #_call, __FILE__, __LINE__, error);              \
                    goto end;                                               \
                }                                                           \
            } while (0)

#define TZ_ASSERT_NOTNULL(_x) TZ_ASSERT(EXC_MEMORY_ERROR, (_x) != NULL)

// Crashes can be harder to debug than exceptions and
// latency isn't a big concern
static inline void check_null(void volatile const *const ptr) {
    if (ptr == NULL) {
        THROW(EXC_MEMORY_ERROR);
    }
}

#ifdef TEZOS_DEBUG
static inline __attribute((noreturn)) void failwith(const char* message) {
  PRINTF("[ERROR] %s\n", message);
  THROW(EXC_UNEXPECTED_STATE);
}
#else
static inline __attribute((noreturn)) void failwith(__attribute((unused))
                                                    const char* message) {
  THROW(EXC_UNEXPECTED_STATE);
}
#endif
