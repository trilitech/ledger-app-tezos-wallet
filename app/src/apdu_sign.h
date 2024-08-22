/* Tezos Ledger application - Clear signing command handler

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 TriliTech <contact@trili.tech>

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

#include <unistd.h>
#include <stdbool.h>

#include <parser.h>

#include "keys.h"
#include "parser/parser_state.h"

/**
 * @brief Save hash of the transaction to be signed.
 *
 */
typedef struct {
    cx_blake2b_t state;  /// Ledger-sdk blake2b state containing hash header
                         /// and blake2b state info.
    uint8_t final_hash[SIGN_HASH_SIZE];  /// Final hash of the transaction.
} apdu_hash_state_t;

/**
 * @brief Represents state of sign transaction.
 *
 */
typedef enum {
    SIGN_ST_IDLE,            /// IDLE
    SIGN_ST_WAIT_DATA,       /// Waiting for more data from apdu interface
    SIGN_ST_WAIT_USER_INPUT  /// Waiting for user action
} sign_step_t;

/**
 * @brief Steps in a blind signing of a transaction.
 *
 */
typedef enum {
    BLINDSIGN_ST_OPERATION,
    BLINDSIGN_ST_HASH,
    BLINDSIGN_ST_ACCEPT_REJECT,
} blindsign_step_t;

/**
 * @brief Steps to display summary.
 *
 */
typedef enum {
    SUMMARYSIGN_ST_OPERATION,
    SUMMARYSIGN_ST_NB_TX,
    SUMMARYSIGN_ST_AMOUNT,
    SUMMARYSIGN_ST_FEE,
    SUMMARYSIGN_ST_HASH,
    SUMMARYSIGN_ST_ACCEPT_REJECT,
} summarysign_step_t;

/**
 * @brief Struct to track state/info about current sign operation.
 *
 */
typedef struct {
    uint8_t packet_index;  /// Index of the packet currently being processed.

    sign_step_t step;  /// Current step of the sign operation.
    bool return_hash;  /// Whether to return the hash of the transaction.
    bool received_last_msg;  /// Whether the last message has been received.
    uint8_t tag;             /// Type of tezos operation to sign.

    union {
        /// @brief clear signing state info.
        struct {
            tz_parser_state parser_state;
            size_t          total_length;
            uint8_t         last_field_index;
            uint8_t         screen_displayed;
            bool            received_msg;
        } clear;
        /// @brief blindsigning state info.
        struct {
            blindsign_step_t step;
        } blind;
        struct {
            summarysign_step_t step;
        } summary;
    } u;
} apdu_sign_state_t;
