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

#include <string.h>

#include "exception.h"

#define COMPARE(a, b)                                                         \
    ({                                                                        \
        _Static_assert(sizeof(*a) == sizeof(*b), "Size mismatch in COMPARE"); \
        check_null(a);                                                        \
        check_null(b);                                                        \
        memcmp(a, b, sizeof(*a));                                             \
    })
#define NUM_ELEMENTS(a) (sizeof(a) / sizeof(*a))

// Macro that produces a compile-error showing the sizeof the argument.
#define ERROR_WITH_NUMBER_EXPANSION(x) char(*__kaboom)[x] = 1;
#define SIZEOF_ERROR(x)                ERROR_WITH_NUMBER_EXPANSION(sizeof(x));
