/* Tezos Ledger application - Public key command handler (visual and silent)

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

#include <string.h>

#include <cx.h>
#include <io.h>
#include <os.h>
#include <ux.h>

#include "apdu.h"
#include "globals.h"
#include "keys.h"

/* Prototypes */

static cx_err_t provide_pubkey(void);
static cx_err_t prompt_address(void);
static cx_err_t format_pkh(char *);
static void stream_cb(tz_ui_cb_type_t);

static tz_err_t provide_pubkey(void) {
  uint8_t io_buffer[128];  /* XXXrcd: fixed length, fix it */
  cx_ecfp_public_key_t pubkey;
  TZ_PREAMBLE(("io_buffer=%p", io_buffer));

  TZ_ASSERT_NOTNULL(io_buffer);
  // Application could be PIN-locked, and pubkey->W_len would then be 0,
  // so throwing an error rather than returning an empty key
  if (os_global_pin_is_validated() != BOLOS_UX_OK) {
    THROW(EXC_SECURITY);
  }
  CX_CHECK(generate_public_key(&pubkey, global.path_with_curve.derivation_type,
                               &global.path_with_curve.bip32_path));
  /* XXXrcd: NO NO NO NO NO !!! */
  io_buffer[0] = pubkey.W_len;
  memmove(io_buffer + 1, pubkey.W, pubkey.W_len);
  io_send_response_pointer(io_buffer, pubkey.W_len + 1, SW_OK);
  /* XXXrcd: huh?!? */

  TZ_POSTAMBLE;
}

static cx_err_t format_pkh(char *buffer) {
  cx_ecfp_public_key_t pubkey = {0};
  uint8_t hash[21];
  TZ_PREAMBLE(("buffer=%p", buffer));

  CX_CHECK(generate_public_key(&pubkey, global.path_with_curve.derivation_type,
                               &global.path_with_curve.bip32_path));
  CX_CHECK(public_key_hash(hash+1, 20, NULL,
                           global.path_with_curve.derivation_type, &pubkey));
  switch (global.path_with_curve.derivation_type) {
  case DERIVATION_TYPE_SECP256K1: hash[0] = 1; break;
  case DERIVATION_TYPE_SECP256R1: hash[0] = 2; break;
  case DERIVATION_TYPE_ED25519:
  case DERIVATION_TYPE_BIP32_ED25519: hash[0] = 0; break;
  default: CX_CHECK(EXC_WRONG_PARAM); break;
  }
  tz_format_pkh(hash, 21, buffer);

  TZ_POSTAMBLE;
}

static void stream_cb(tz_ui_cb_type_t type) {

  switch (type) {
  case TZ_UI_STREAM_CB_ACCEPT:
    provide_pubkey();       /* XXXrcd: not much to do on failure */
    global.step = ST_IDLE;
    break;
  case TZ_UI_STREAM_CB_REFILL:
    break;
  case TZ_UI_STREAM_CB_REJECT:
    io_send_sw(EXC_REJECT);
    global.step = ST_IDLE;
    break;
  }
}

#ifdef HAVE_BAGL
static cx_err_t prompt_address(void) {
  char buf[TZ_UI_STREAM_CONTENTS_SIZE + 1];
  TZ_PREAMBLE(("void"));

  tz_ui_stream_init(stream_cb);
  CX_CHECK(format_pkh(buf));
  tz_ui_stream_push_all(TZ_UI_STREAM_CB_NOCB, "Provide Key", buf,
                        TZ_UI_ICON_NONE);
  tz_ui_stream_push_accept_reject();
  tz_ui_stream_close();
  tz_ui_stream();

  TZ_POSTAMBLE;
}
#elif HAVE_NBGL

#include "nbgl_use_case.h"

static void cancel_callback(void) {
  stream_cb(TZ_UI_STREAM_CB_REJECT);
  nbgl_useCaseStatus("Address rejected", false, ui_home_init);
}

static void approve_callback(void) {
  stream_cb(TZ_UI_STREAM_CB_ACCEPT);
  nbgl_useCaseStatus("ADDRESS\nVERIFIED", true, ui_home_init);
}

static void confirmation_callback(bool confirm) {
  if (confirm) {
    approve_callback();
  } else {
    cancel_callback();
  }
}

static void verify_address(void) {
  char buf[TZ_UI_STREAM_CONTENTS_SIZE + 1];
  CX_THROW(format_pkh(buf));

  nbgl_useCaseAddressConfirmation(buf, confirmation_callback);
}

static cx_err_t prompt_address(void) {
  FUNC_ENTER(("void"));
  nbgl_useCaseReviewStart(&C_tezos, "Verify Tezos\naddress", NULL, "Cancel", verify_address, cancel_callback);
  // THROW(ASYNC_EXCEPTION);
}
#endif

tz_err_t handle_apdu_get_public_key(command_t *cmd) {
  bool prompt = cmd->ins == INS_PROMPT_PUBLIC_KEY;
  TZ_PREAMBLE(("cmd=%p", cmd));

  TZ_ASSERT(EXC_UNEXPECTED_STATE, global.step == ST_IDLE);
  TZ_ASSERT(EXC_WRONG_PARAM, cmd->p1 == 0);

  // do not expose pks without prompt through U2F (permissionless legacy
  // comm in browser)
  TZ_ASSERT(EXC_HID_REQUIRED, prompt || G_io_apdu_media != IO_APDU_MEDIA_U2F);

  global.path_with_curve.derivation_type = cmd->p2;
  CX_CHECK(check_derivation_type(global.path_with_curve.derivation_type));
  CX_CHECK(read_bip32_path(&global.path_with_curve.bip32_path, cmd->data,
                           cmd->lc));
  if (!prompt) {
    error = provide_pubkey();
    goto end;
  }

  global.step = ST_PROMPT;
  prompt_address();

  TZ_POSTAMBLE;
}
