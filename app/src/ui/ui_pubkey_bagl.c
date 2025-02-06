/* Tezos Ledger application - Public key review display for Bagl

   Copyright 2025 Functori <contact@functori.com>
   Copyright 2024 TriliTech <contact@trili.tech>
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

#ifdef HAVE_BAGL
#include "globals.h"

#include "ui_pubkey.h"

#define G_pubkey global.ui.pubkey

/**
 * @brief Public key review callback
 *
 * @param confirm: whether the user accept or reject the public key
 */
static void
ux_pubkey_callback(bool confirm)
{
    FUNC_ENTER(("confirm=%d", confirm));

    (*G_pubkey.callback)(confirm);

    global.step = ST_IDLE;
    ui_home_init();

    FUNC_LEAVE();
}

// clang-format off
#ifdef TARGET_NANOS
UX_STEP_NOCB(ux_pubkey_review_step, pb,
           {&C_icon_eye, "Verify address"});
#else
UX_STEP_NOCB(ux_pubkey_review_step, pbb,
           {&C_icon_eye, "Verify", "address"});
#endif
UX_STEP_NOCB(ux_pubkey_address_step, bn_paging,
           {"Address", G_pubkey.address});
UX_STEP_CB(ux_pubkey_confirm_step, pb, ux_pubkey_callback(true),
           {&C_icon_validate_14, "Approve"});
UX_STEP_CB(ux_pubkey_reject_step, pb, ux_pubkey_callback(false),
           {&C_icon_crossmark, "Reject"});

UX_FLOW(ux_pubkey_review_flow,
        &ux_pubkey_review_step,
        &ux_pubkey_address_step,
        &ux_pubkey_confirm_step,
        &ux_pubkey_reject_step);
// clang-format on

void
ui_pubkey_review(cx_ecfp_public_key_t *pubkey,
                 derivation_type_t     derivation_type,
                 action_validate_cb    callback)
{
    TZ_PREAMBLE(("pubkey=%p, derivation_type=%d, callback=%p", pubkey,
                 derivation_type, callback));

    global.step = ST_PROMPT;

    G_pubkey.callback = callback;
    TZ_LIB_CHECK(derive_pkh(pubkey, derivation_type, G_pubkey.address,
                            sizeof(G_pubkey.address)));

    ux_flow_init(0, ux_pubkey_review_flow, NULL);

    TZ_POSTAMBLE;
}
#endif
