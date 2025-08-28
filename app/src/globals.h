/* Tezos Ledger application - Global application state

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Trilitech <contact@trili.tech>

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
#include <memory.h>
#include <string.h>
#include <bolos_target.h>

#define TZ_SCREEN_WITDH_FULL_REGULAR_11PX       19
#define TZ_SCREEN_WITDH_BETWEEN_ICONS_BOLD_11PX 16
#ifdef TARGET_NANOS
#define TZ_SCREEN_LINES_11PX 2
#else
#define TZ_SCREEN_LINES_11PX 5
#endif

#include "sign.h"
#include "exception.h"
#include "keys.h"
#include "ui_commons.h"
#include "ui_stream.h"
#include "ui_home.h"
#include "ui_settings.h"
#include "ui_pubkey.h"
#include "utils.h"

#ifdef HAVE_BAGL
#include "ui_settings.h"
#endif

#include "parser/parser_state.h"

#define MAX_APDU_SIZE      235
#define MAX_SIGNATURE_SIZE 100
#define ERROR_CODE_SIZE    15

/**
 * @brief State of the app
 *
 */
typedef enum {
    ST_IDLE,          /// Idle state
    ST_CLEAR_SIGN,    /// Clear signing an operation
    ST_BLIND_SIGN,    /// Blind signing an operation
    ST_SUMMARY_SIGN,  /// Summary signing an operation
    ST_PROMPT,        /// Waiting for user prompt
    ST_SWAP_SIGN,     /// Performing swap operations
    ST_ERROR          /// In error state.
} main_step_t;

#ifdef TARGET_NANOS
#define NB_MAX_SCREEN_ALLOWED 20
#elif defined(HAVE_BAGL)
#define NB_MAX_SCREEN_ALLOWED 12
#endif

#ifdef HAVE_NBGL
typedef enum {
    REASON_NONE          = 0,
    REASON_PARSING_ERROR = 1
} blindsign_reason_t;
#endif

/**
 * @brief Global structure holding state of operations and buffer of the data
 * to be processed.
 *
 */
typedef struct {
    // UI structs
    union {
        tz_ui_stream_t stream;  /// stream UX and UI related information
#ifdef HAVE_BAGL
        tz_ui_settings_t settings;  /// settings UI buffers
#endif
        tz_ui_pubkey_t
            pubkey;  /// UX and UI information related to public key
    } ui;
    bip32_path_with_curve_t path_with_curve;  /// Derivation path
    union {
        struct {
            apdu_hash_state_t hash;  /// Transaction hash
            apdu_sign_state_t sign;  ///  state of sign operation.
        } apdu;
        /** Warning: Use this pubkey only when apdu-hash/sign
         * is not being used.
         * Only being used in apdu_pubkey.c : handle_apdu_get_public_key
         * currently.
         * */
        cx_ecfp_public_key_t pubkey;
    } keys;
    /// Buffer to store incoming data.
    char line_buf[TZ_UI_STREAM_CONTENTS_SIZE + 1];

#ifdef HAVE_NBGL
    blindsign_reason_t
         blindsign_reason;  /// Blindsigning flow Summary or parsing error.
    char error_code[ERROR_CODE_SIZE];  /// Error code for parsing error.
#endif
    main_step_t step;  /// Current operational state of app.
} globals_t;

/* Settings */
typedef struct {
    bool expert_mode;   /// enable expert mode
    bool blindsigning;  /// Blindsign status
} settings_t;           /// Special settings available in the app.

extern globals_t global;

extern const settings_t N_settings_real;
#define N_settings (*(volatile settings_t *)PIC(&N_settings_real))

extern unsigned int app_stack_canary;  // From SDK
/**
 * @brief IO buffer.
 *
 */
extern unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

/**
 * @brief  Zeros out all application-specific globals and SDK-specific
 * UI/exchange buffers.
 */
void init_globals(void);

/// Toggles the persisted expert_mode setting
void toggle_expert_mode(void);

/// Toggles the persisted blindsign setting between "ON",
/// "OFF".
void toggle_blindsigning(void);
