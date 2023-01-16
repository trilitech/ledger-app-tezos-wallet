#pragma once

// Some "good enough" heuristics to know if we are actually compiling
// for BOLOS or running in the Tezos client/testing.
#if __has_include("os.h")
#include "os.h"
#  if __has_include("cx.h")
#    define ACTUALLY_ON_LEDGER
#  endif
#endif

// On BOLOS, use the SDK.
// Elsewhere, simulate the BOLOS API calls we need.
#ifdef ACTUALLY_ON_LEDGER
#  include "os.h"
#  include "os_io_seproxyhal.h"
#  include "cx.h"
#  if CX_APILEVEL < 8
#    error "May only compile with API level 8 or higher; requires newer firmware"
#  endif
#else
#  include <stdio.h>
#  define PIC(x) ((void*) x)
#  define PRINTF printf
#endif

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Type-safe versions of true/false
#undef true
#define true ((bool) 1)
#undef false
#define false ((bool) 0)
