#pragma once

#include "globals.h"

/*
 * Debugging macros.
 *
 * We may decide in the future to make most of FUNC_ENTER into a function
 * to stop `uint8_t _tmp` being allocated on the function's stack.
 *
 * We output the function's name on the `stack =` line to allow one to
 * perform magic with grep, sort, and uniq -c.
 */

#ifdef TEZOS_DEBUG
#define FUNC_ENTER(x)	do {						\
                uint8_t _tmp;						\
                PRINTF("[DEBUG] call %s(", __func__);			\
                PRINTF x;						\
                PRINTF(") at %s:%u\n", __FILE__, __LINE__);		\
                PRINTF("[DEBUG] stack = 0x%p (%s)\n", &_tmp, __func__);	\
                if (app_stack_canary != 0xDEADBEEF)			\
                    PRINTF("[DEBUG] Stack (0x%p) has been smashed\n",	\
                           &app_stack_canary);				\
        } while (0)
#define FUNC_LEAVE()	do {						\
                if (app_stack_canary != 0xDEADBEEF)			\
                    PRINTF("[DEBUG] Stack (0x%p) has been smashed "	\
                           "(leaving function)\n", &app_stack_canary);	\
                PRINTF("[DEBUG] leave %s\n", __func__);			\
        } while (0)
#else
#define FUNC_ENTER(x)
#define FUNC_LEAVE()
#endif
