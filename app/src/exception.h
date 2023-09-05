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

#define SW_OK	0x9000

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
#define EXC_UNKNOWN_CX_ERR            0x9003
#define EXC_UNKNOWN                   0x90FF

/*
 * In the handlers and the routines that they call, we define a set of
 * macros to standardise them, ease writing, and reduce repetition.  We
 * begin with TZ_PREAMBLE().  It takes a bracketted list of PRINTF()
 * arguments analogous the FUNC_ENTER() which it calls with said list.
 * It also defines local variables for error handing.  TZ_PREAMBLE()
 * must be placed after all of the variable definitions and before
 * the code if you are using C89 or prior. Otherwise, it is sufficient
 * that it is before any call to TZ_ASSERT(), TZ_CHECK(), CX_CHECK(), etc.
 *
 * TZ_CHECK() has similar calling conventions to CX_CHECK, but it uses
 * different variables and that's because the meaning is different.  The
 * former uses a tz_err_t which should correspond directly to an SW sent
 * back to a client if it is non-zero when percolated all the way back
 * up to main.  CX_CHECK() is caught in TZ_POSTAMBLE and converted into
 * a tz_err_t.
 *
 * TZ_POSTAMBLE defines the labels to which {CX,TZ}_CHECK() and
 * TZ_ASSERT() jump.  They are ref'd in TZ_POSTAMBLE() to suppress
 * warnings. TZ_POSTAMBLE also calls FUNC_LEAVE to mirror the preamble
 * calling FUNC_ENTER().
 */

typedef uint32_t tz_err_t;
#define TZ_OK		0x0000

#define TZ_PREAMBLE(_args)   						\
		tz_err_t _tz_ret_code = TZ_OK;				\
		cx_err_t error = CX_OK;					\
		if (0)							\
		    goto bail;						\
		if (0)							\
		    goto end;						\
		FUNC_ENTER(_args)

#define TZ_POSTAMBLE		  					\
	    end:							\
		if (error != CX_OK) {					\
	            _tz_ret_code = EXC_UNKNOWN_CX_ERR;			\
		    PRINTF("CX_CHECK failed with 0x%08x", error);	\
		}							\
	    bail:							\
		FUNC_LEAVE();						\
		return _tz_ret_code

#define TZ_ASSERT(_err, _cond) do {                                         \
                if (!(_cond)) {                                             \
                    PRINTF("Assertion (\"%s\") on %s:%u failed with %s\n",  \
                           #_cond, __FILE__, __LINE__, #_err);              \
                    _tz_ret_code = (_err);                                  \
                    goto bail;                                              \
                }                                                           \
            } while (0)

#define TZ_CHECK(_call) do {                                                \
                _tz_ret_code = (_call);                                     \
                if (_tz_ret_code) {                                         \
                    PRINTF("TZ_CHECK(\"%s\") on %s:%u failed with %u\n",    \
                           #_call, __FILE__, __LINE__, _tz_ret_code);       \
                    goto bail;                                              \
                }                                                           \
            } while (0)

#define TZ_ASSERT_NOTNULL(_x) TZ_ASSERT(EXC_MEMORY_ERROR, (_x) != NULL)

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
