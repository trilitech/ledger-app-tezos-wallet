/* Tezos Ledger application - Public key command handler (visual and silent)

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>

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

#include <string.h>

#include <buffer.h>
#include <cx.h>
#include <io.h>
#include <os.h>
#include <ux.h>

#include "apdu.h"

#include "compat.h"
#include "globals.h"
#include "keys.h"
#include "ui_pubkey.h"

/**
 * @brief Sends the public key on confirm
 *        Sends the rejects response otherwise
 *
 * @param confirm: if sends the public key or the rejects response
 */
static void
send_pubkey_response(bool confirm)
{
    FUNC_ENTER(("confirm=%d", confirm));

    if (confirm) {
        buffer_t bufs[2] = {
            {.ptr    = (const uint8_t *)&global.keys.pubkey.W_len,
             .size   = 1,
             .offset = 0u},
            {.ptr    = global.keys.pubkey.W,
             .size   = global.keys.pubkey.W_len,
             .offset = 0u},
        };
        io_send_response_buffers(bufs, 2, SW_OK);
    } else {
        io_send_sw(EXC_REJECT);
    }

    memset(&global.keys, 0, sizeof(global.keys));

    FUNC_LEAVE();
}

void
handle_get_public_key(buffer_t *cdata, derivation_type_t derivation_type,
                      bool prompt)
{
    TZ_PREAMBLE(("cdata=%p, derivation_type=%d, prompt=%d", cdata,
                 derivation_type, prompt));

    global.path_with_curve.derivation_type = derivation_type;
    TZ_LIB_CHECK(read_bip32_path(&global.path_with_curve.bip32_path, cdata));

    // Derive public key and store it on global.keys.pubkey
    TZ_LIB_CHECK(derive_pk(&global.keys.pubkey,
                           global.path_with_curve.derivation_type,
                           &global.path_with_curve.bip32_path));

    if (prompt) {
        ui_pubkey_review(&global.keys.pubkey,
                         global.path_with_curve.derivation_type,
                         &send_pubkey_response);
    } else {
        send_pubkey_response(true);
    }

    TZ_POSTAMBLE;
}
