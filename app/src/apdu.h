/* Tezos Ledger application - Some common primitives and some command handlers

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

#include <stdbool.h>
#include <string.h>

#include <buffer.h>

#include "exception.h"
#include "keys.h"

// Instruction codes
#define INS_VERSION                   0x00
#define INS_AUTHORIZE_BAKING          0x01
#define INS_GET_PUBLIC_KEY            0x02
#define INS_PROMPT_PUBLIC_KEY         0x03
#define INS_SIGN                      0x04
#define INS_SIGN_UNSAFE               0x05  // Data that is already hashed.
#define INS_RESET                     0x06
#define INS_QUERY_AUTH_KEY            0x07
#define INS_QUERY_MAIN_HWM            0x08
#define INS_GIT                       0x09
#define INS_SETUP                     0x0A
#define INS_QUERY_ALL_HWM             0x0B
#define INS_DEAUTHORIZE               0x0C
#define INS_QUERY_AUTH_KEY_WITH_CURVE 0x0D
#define INS_HMAC                      0x0E
#define INS_SIGN_WITH_HASH            0x0F

// Last valid instruction code
#define INS_MAX 0x0F

/*
 * All of the handlers must be defined together below.  We
 * define them together as they are required to present the
 * same interface and it's easier to validate if they are
 * in the same place.
 *
 * Handlers do not return a value, rather they set global.step
 * to ST_ERROR on errors.  We do this because we need a unified
 * error handling regime which also works with callbacks from
 * the UI code which has no way to percolate return values.
 *
 * Handlers frequently need to update the global state.  They
 * should do this by directly setting global.step.  Each handler
 * should:
 *
 *     1. if global.step == ST_IDLE, this is first time that the
 *        handler is being called and so all handler specific memory
 *        should be initialised,
 *
 *     2. the handler should assert that it is in the correct state.
 *
 * Note that (1) occurs before (2).  This is because if we are back in
 * ST_IDLE, that means that a previous invocation of this handler or a
 * callback in the UI bits has invalidated any existing transaction that
 * is in flight.  Our variables might be a union with other handlers and
 * all zeroes may not be the default state for any handler and so there
 * is no reliable way for any handler to reset its memory to a factory
 * default setting for another unspecified future handler.  And so, we
 * have each handler reset its data structures up front where we know
 * what the next handler is going to be.
 *
 */

/**
 * @brief Handle version request.
 * Send APDU response containing the version.
 */
void handle_version(void);

/**
 * @brief Handle git commit request.
 * Send APDU response containing the git commit.
 */
void handle_git(void);

/**
 * @brief Handle public key request.
 * If successfully parse BIP32 path, send APDU response containing the public
 * key.
 *
 * The public key is derived only once and stored in the RAM, in order to
 * avoid repeated derivation calculations.
 *
 * @param cdata: buffer containing the BIP32 path of the key
 * @param derivation_type: derivation_type of the key
 * @param prompt: whether to display address on screen or not
 */
void handle_get_public_key(buffer_t *cdata, derivation_type_t derivation_type,
                           bool prompt);

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
