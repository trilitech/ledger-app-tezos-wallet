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

#include <cx.h>
#include <io.h>
#include <os.h>
#include <ux.h>

#include "apdu.h"
#include "compat.h"
#include "globals.h"
#include "keys.h"

/* Prototypes */

static void provide_pubkey(void);
static void prompt_address(void);
static void format_pkh(cx_ecfp_public_key_t *, char *, size_t);
static void stream_cb(tz_ui_cb_type_t);

static void
provide_pubkey(void)
{
    buffer_t bufs[2] = {0};
    uint8_t  byte;
    TZ_PREAMBLE(("void"));

    byte         = global.keys.pubkey.W_len;
    bufs[0].ptr  = &byte;
    bufs[0].size = 1;
    bufs[1].ptr  = global.keys.pubkey.W;
    bufs[1].size = global.keys.pubkey.W_len;
    io_send_response_buffers(bufs, 2, SW_OK);
    global.step = ST_IDLE;

    TZ_POSTAMBLE;
}

static void
format_pkh(cx_ecfp_public_key_t *pubkey, char *buffer, size_t len)
{
    derive_pkh(pubkey, global.path_with_curve.derivation_type, buffer, len);
}

static void
stream_cb(tz_ui_cb_type_t cb_type)
{
    TZ_PREAMBLE(("cb_type=%u", cb_type));

    // clang-format off
    switch (cb_type) {
    case TZ_UI_STREAM_CB_ACCEPT: TZ_CHECK(provide_pubkey()); break;
    case TZ_UI_STREAM_CB_REFILL:                             break;
    case TZ_UI_STREAM_CB_REJECT: TZ_FAIL(EXC_REJECT);        break;
    }
    // clang-format on

    // clear the public key from global storage after operation.
    memset(&global.keys, 0, sizeof(global.keys));
    TZ_POSTAMBLE;
}

#ifdef HAVE_BAGL
static void
prompt_address(void)
{
    char buf[TZ_BASE58CHECK_BUFFER_SIZE(20, 3)];
    TZ_PREAMBLE(("void"));

    global.step = ST_PROMPT;
    tz_ui_stream_init(stream_cb);
    TZ_CHECK(format_pkh(&global.keys.pubkey, buf, sizeof(buf)));
#ifdef TARGET_NANOS
    tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Verify address", "",
                      TZ_UI_LAYOUT_HOME_PB, TZ_UI_ICON_EYE);
#else
    tz_ui_stream_push(TZ_UI_STREAM_CB_NOCB, "Verify", "address",
                      TZ_UI_LAYOUT_HOME_PB, TZ_UI_ICON_EYE);
#endif
    tz_ui_stream_push_all(TZ_UI_STREAM_CB_NOCB, "Provide Key", buf,
                          TZ_UI_LAYOUT_BNP, TZ_UI_ICON_NONE);
    tz_ui_stream_push(TZ_UI_STREAM_CB_ACCEPT, "Approve", "",
                      TZ_UI_LAYOUT_HOME_PB, TZ_UI_ICON_TICK);
    tz_ui_stream_push(TZ_UI_STREAM_CB_REJECT, "Reject", "",
                      TZ_UI_LAYOUT_HOME_PB, TZ_UI_ICON_CROSS);
    tz_ui_stream_close();
    tz_ui_stream();

    TZ_POSTAMBLE;
}
#elif HAVE_NBGL

#include "nbgl_use_case.h"

static void
cancel_callback(void)
{
    stream_cb(TZ_UI_STREAM_CB_REJECT);
    global.step = ST_IDLE;
    nbgl_useCaseStatus("Address rejected", false, ui_home_init);
}

static void
approve_callback(void)
{
    stream_cb(TZ_UI_STREAM_CB_ACCEPT);
    nbgl_useCaseStatus("ADDRESS\nVERIFIED", true, ui_home_init);
}

static void
confirmation_callback(bool confirm)
{
    if (confirm) {
        approve_callback();
    } else {
        cancel_callback();
    }
}

static void
verify_address(void)
{
    TZ_PREAMBLE(("void"));

    TZ_CHECK(format_pkh(&global.keys.pubkey, global.stream.verify_address,
                        sizeof(global.stream.verify_address)));
    nbgl_useCaseAddressConfirmation(global.stream.verify_address,
                                    confirmation_callback);
    TZ_POSTAMBLE;
}

static void
prompt_address(void)
{
    TZ_PREAMBLE(("void"));

    global.step = ST_PROMPT;
    nbgl_useCaseReviewStart(&C_tezos, "Verify Tezos\naddress", NULL, "Cancel",
                            verify_address, cancel_callback);
    TZ_POSTAMBLE;
}
#endif

void
handle_apdu_get_public_key(command_t *cmd)
{
    bool prompt = cmd->ins == INS_PROMPT_PUBLIC_KEY;
    TZ_PREAMBLE(("cmd=%p", cmd));

    TZ_ASSERT(EXC_UNEXPECTED_STATE, global.step == ST_IDLE);
    TZ_ASSERT(EXC_WRONG_PARAM, cmd->p1 == 0);

    // do not expose pks without prompt through U2F (permissionless legacy
    // comm in browser)
    TZ_ASSERT(EXC_HID_REQUIRED,
              prompt || G_io_apdu_media != IO_APDU_MEDIA_U2F);

    global.path_with_curve.derivation_type = cmd->p2;
    TZ_ASSERT(EXC_WRONG_PARAM,
              check_derivation_type(global.path_with_curve.derivation_type));
    TZ_CHECK(read_bip32_path(&global.path_with_curve.bip32_path, cmd->data,
                             cmd->lc));

    // Derive public key and store it on global.keys.pubkey

    TZ_CHECK(derive_pk(&global.keys.pubkey,
                       global.path_with_curve.derivation_type,
                       &global.path_with_curve.bip32_path));
    if (prompt)
        prompt_address();
    else
        provide_pubkey();

    TZ_POSTAMBLE;
}
