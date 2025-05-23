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

#include <buffer.h>

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
#ifdef HAVE_BAGL
            uint8_t screen_displayed;
#endif
            bool received_msg;
            bool displayed_expert_warning;
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

/**
 * @brief Handle signing key setup request.
 * If successfully parse BIP32 path, set up the key as the signing key,
 * initialize the signing state and send validation APDU response.
 *
 * @param cdata: data containing the BIP32 path of the key
 * @param derivation_type: derivation_type of the key
 * @param return_hash: whether the hash of the message is requested or not
 */
void handle_signing_key_setup(buffer_t         *cdata,
                              derivation_type_t derivation_type,
                              bool              return_hash);

/**
 * @brief Handle operation/micheline expression signature request.
 *
 * Parse the received command and prompt user for appropriate
 * action. Triggers blindsigning and/or expert mode workflows based on
 * transaction involved. Stream based parser helps decode arbitararily
 * large transaction, screen by screen. After user validation, sign the hashed
 * message and send an ADPU response containing the signature.
 *
 * @param cdata: data containing the message to sign
 * @param last: whether the part of the message is the last one or not
 * @param with_hash: whether the hash of the message is requested or not
 */
void handle_sign(buffer_t *cdata, bool last, bool return_hash);
