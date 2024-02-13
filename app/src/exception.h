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

#include <io.h>
#include <os.h>

#define SW_OK 0x9000u

// Standard APDU error codes:
// https://www.eftlab.com/knowledge-base/complete-list-of-apdu-responses
// https://docs.zondax.ch/ledger-apps/starkware/APDU

typedef uint16_t tz_exc;

#define EXC_WRONG_PARAM               0x6B00u
#define EXC_WRONG_LENGTH              0x6C00u
#define EXC_INVALID_INS               0x6D00u
#define EXC_WRONG_LENGTH_FOR_INS      0x917Eu
#define EXC_REJECT                    0x6985u
#define EXC_PARSE_ERROR               0x9405u
#define EXC_REFERENCED_DATA_NOT_FOUND 0x6A88u
#define EXC_WRONG_VALUES              0x6A80u
#define EXC_SECURITY                  0x6982u
#define EXC_HID_REQUIRED              0x6983u
#define EXC_CLASS                     0x6E00u
#define EXC_MEMORY_ERROR              0x9200u

#define EXC_UNEXPECTED_STATE      0x9001u
#define EXC_UNEXPECTED_SIGN_STATE 0x9002u
#define EXC_UNKNOWN_CX_ERR        0x9003u
#define EXC_UNKNOWN               0x90FFu

/*
 * In the handlers and the routines that they call, we define a set of
 * macros to standardise them, ease writing, and reduce repetition.
 *
 * Each function returns void.  We do this because we have to have the
 * same logic in callbacks from the UI code which may also send responses.
 * And so, to keep things the same we assume that all of our handlers
 * can't return values, either, and do things the same way in both places.
 * Our mechanism is to set global.step == ST_ERROR after we get an error.
 *
 * Each function should begin with TZ_PREAMBLE() which takes a
 * bracketted list of PRINTF() arguments.  It also sets up all of
 * the variables necessary for the following macros.
 *
 * Each function should end with TZ_POSTAMBLE which: sets up all of
 * labels to which the remaining macros jump; responds on error via
 * io_send_sw(); and sets global.step on error.
 *
 * TZ_CHECK() calls a function.  If global.step == ST_ERROR, it will
 * jump to the end after calling the function and let TZ_POSTAMBLE do
 * its thing.
 *
 * TZ_FAIL(sw_code) will set vars and jump into the postamble which
 * will reply using io_send_sw(sw_code) and set global.step = ST_ERROR.
 *
 * TZ_ASSERT(sw_code, condition) will TZ_FAIL(sw_code) if the condition
 * is not true.
 *
 * CX_CHECK() is a macro provided by BOLOS, however, we catch it
 * with this framework, reply with io_send_sw() and return TZ_DONE.
 */

#define TZ_PREAMBLE(_args)           \
    tz_exc   _sw_ret_code = 0x0000u; \
    cx_err_t error        = CX_OK;   \
    if (error != CX_OK) {            \
        goto bail;                   \
    }                                \
    if (0) {                         \
        goto end;                    \
    }                                \
    FUNC_ENTER(_args)

#define TZ_POSTAMBLE                                  \
    end:                                              \
    if (error != CX_OK) {                             \
        _sw_ret_code = EXC_UNKNOWN_CX_ERR;            \
        PRINTF("CX_CHECK failed with 0x%08x", error); \
    }                                                 \
    bail:                                             \
    if (_sw_ret_code) {                               \
        global.step = ST_ERROR;                       \
        io_send_sw(_sw_ret_code);                     \
    }                                                 \
    FUNC_LEAVE();

#define TZ_LIB_POSTAMBLE                              \
    end:                                              \
    if (error != CX_OK) {                             \
        _sw_ret_code = EXC_UNKNOWN_CX_ERR;            \
        PRINTF("CX_CHECK failed with 0x%08x", error); \
    }                                                 \
    bail:                                             \
    FUNC_LEAVE();                                     \
    return _sw_ret_code

#define TZ_FAIL(_sw_code)        \
    do {                         \
        _sw_ret_code = _sw_code; \
        goto bail;               \
    } while (0)

#define TZ_SUCCEED() \
    do {             \
        goto bail;   \
    } while (0)

#define TZ_ASSERT(_err, _cond)                                             \
    do {                                                                   \
        if (!(_cond)) {                                                    \
            PRINTF("Assertion (\"%s\") on %s:%u failed with %s\n", #_cond, \
                   __FILE__, __LINE__, #_err);                             \
            TZ_FAIL(_err);                                                 \
        }                                                                  \
    } while (0)

#define TZ_CHECK(_call)                                             \
    do {                                                            \
        (_call);                                                    \
        if (global.step == ST_ERROR) {                              \
            PRINTF("TZ_CHECK(\"%s\") on %s:%u\n", #_call, __FILE__, \
                   __LINE__);                                       \
            goto bail;                                              \
        }                                                           \
    } while (0)

#define TZ_LIB_CHECK(_call)                                             \
    do {                                                                \
        _sw_ret_code = (_call);                                         \
        if (_sw_ret_code) {                                             \
            PRINTF("TZ_LIB_CHECK(\"%s\") on %s:%u\n", #_call, __FILE__, \
                   __LINE__);                                           \
            goto bail;                                                  \
        }                                                               \
    } while (0)

#define TZ_ASSERT_NOTNULL(_x) TZ_ASSERT(EXC_MEMORY_ERROR, (_x) != NULL)
