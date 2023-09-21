/* Tezos Embedded C parser for Ledger - Platform compatibility layer

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

// Some "good enough" heuristics to know if we are actually compiling
// for BOLOS or running in the Tezos client/testing.
#if __has_include("os.h")
#include "os.h"
#if __has_include("cx.h")
#define ACTUALLY_ON_LEDGER
#endif
#endif

// On BOLOS, use the SDK.
// Elsewhere, simulate the BOLOS API calls we need.
#ifdef ACTUALLY_ON_LEDGER
#include "os.h"
#include "os_io_seproxyhal.h"
#include "cx.h"
#if CX_APILEVEL < 8
#error "Only compiles with API level 8 or higher; requires newer firmware"
#endif
#else
#include <stdio.h>
#define PIC(x) ((void *)x)
#ifdef TEZOS_DEBUG
#define PRINTF printf
#else
#define PRINTF(...) \
    do {            \
    } while (0)
#endif
#endif

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Type-safe versions of true/false
#undef true
#define true ((bool)1)
#undef false
#define false ((bool)0)

#ifdef MIN
#undef MIN
#endif
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define STRLCPY(x, y) strlcpy((x), (y), sizeof(x))
