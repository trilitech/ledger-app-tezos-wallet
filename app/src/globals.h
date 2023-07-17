/* Tezos Ledger application - Global application state

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

#include "bolos_target.h"

#include "exception.h"
#include "keys.h"
#include "parser/parser_state.h"
#include "utils.h"

#define TZ_SCREEN_WITDH_FULL_REGULAR_11PX       19
#define TZ_SCREEN_WITDH_BETWEEN_ICONS_BOLD_11PX 16
#ifdef TARGET_NANOS
#  define TZ_SCREEN_LINES_11PX                    3
#else
#  define TZ_SCREEN_LINES_11PX                    5
#endif

#include "ui_commons.h"
#include "ui_stream.h"
#include "ui_home.h"
#include "ui_settings.h"

// Zeros out all globals that can keep track of APDU instruction state.
// Notably this does *not* include UI state.
void clear_apdu_globals(void);

// Zeros out all application-specific globals and SDK-specific UI/exchange buffers.
void init_globals(void);

#define MAX_APDU_SIZE 235  // Maximum number of bytes in a single APDU

// Our buffer must accommodate any remainder from hashing and the next message at once.
#define TEZOS_BUFSIZE (BLAKE2B_BLOCKBYTES + MAX_APDU_SIZE)

#define PRIVATE_KEY_DATA_SIZE 64
#define MAX_SIGNATURE_SIZE 100

typedef enum {
  SCREEN_CLEAR_SIGN,
  SCREEN_BLIND_SIGN,
  SCREEN_SETTINGS,
  SCREEN_QUIT,
} screen_t;

typedef enum {
  ST_IDLE,
  ST_CLEAR_SIGN,
  ST_BLIND_SIGN,
  ST_PROMPT
} main_step_t;

typedef struct {
  cx_blake2b_t state;
  uint8_t final_hash[SIGN_HASH_SIZE];
} apdu_hash_state_t;

typedef enum {
  SIGN_ST_IDLE,
  SIGN_ST_WAIT_DATA,
  SIGN_ST_WAIT_USER_INPUT
} sign_step_t;

typedef struct {
  uint8_t packet_index;

  sign_step_t step;
  bool received_last_msg;

  union {
    struct {
      size_t total_length;
      tz_parser_state parser_state;
    } clear;
    struct {
      uint8_t tag;
    } blind;
  } u; 

  char line_buf[TZ_UI_STREAM_CONTENTS_SIZE + 1];
} apdu_sign_state_t;

typedef struct {
  /* Settings */
  struct {
    bool blindsigning;
  } settings;

  /* State */
  main_step_t step;
  tz_ui_stream_t stream;
  bip32_path_with_curve_t path_with_curve;
  struct {
    apdu_hash_state_t hash;
    apdu_sign_state_t sign;
  } apdu;
# ifdef HAVE_BAGL
  struct {
    bagl_element_t bagls[5 + TZ_SCREEN_LINES_11PX];
    char lines[TZ_SCREEN_LINES_11PX][TZ_SCREEN_WITDH_FULL_REGULAR_11PX + 1];
  } ux;
# endif
} globals_t;

extern globals_t global;

extern unsigned int app_stack_canary;  // From SDK

extern unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

// extern const uint32_t mainnet_chain_id = 0x7A06A770 // NetXdQprcVkpaWU
