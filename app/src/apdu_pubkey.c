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

#include "apdu_pubkey.h"

#include "apdu.h"
#include "cx.h"
#include "globals.h"
#include "keys.h"

#include <string.h>

/* Prototypes */

static size_t provide_pubkey(uint8_t *);
static void format_pkh(char *);
static void stream_cb(tz_ui_cb_type_t);

static size_t provide_pubkey(uint8_t *const io_buffer) {
  cx_ecfp_public_key_t pubkey;
  size_t tx = 0;

  FUNC_ENTER(("io_buffer=%p", io_buffer));
  check_null(io_buffer);
  // Application could be PIN-locked, and pubkey->W_len would then be 0,
  // so throwing an error rather than returning an empty key
  if (os_global_pin_is_validated() != BOLOS_UX_OK) {
    THROW(EXC_SECURITY);
  }
  generate_public_key(&pubkey, global.path_with_curve.derivation_type,
                      &global.path_with_curve.bip32_path);
  io_buffer[tx++] = pubkey.W_len;
  memmove(io_buffer + tx, pubkey.W, pubkey.W_len);
  tx += pubkey.W_len;
  FUNC_LEAVE();
  return finalize_successful_send(tx);
}

static void format_pkh(char* buffer) {
  cx_ecfp_public_key_t pubkey = {0};
  uint8_t hash[21];

  FUNC_ENTER(("buffer=%p", buffer));
  generate_public_key(&pubkey, global.path_with_curve.derivation_type,
                      &global.path_with_curve.bip32_path);
  public_key_hash(hash+1, 20, NULL, global.path_with_curve.derivation_type,
                  &pubkey);
  switch (global.path_with_curve.derivation_type) {
  case DERIVATION_TYPE_SECP256K1: hash[0] = 1; break;
  case DERIVATION_TYPE_SECP256R1: hash[0] = 2; break;
  case DERIVATION_TYPE_ED25519:
  case DERIVATION_TYPE_BIP32_ED25519: hash[0] = 0; break;
  default: THROW(EXC_WRONG_PARAM); // XXX find appropriate error
  }
  tz_format_pkh(hash, 21, buffer);
  FUNC_LEAVE();
}

static void stream_cb(tz_ui_cb_type_t type) {
  switch (type) {
  case TZ_UI_STREAM_CB_ACCEPT:
    delayed_send(provide_pubkey(G_io_apdu_buffer));
    global.step = ST_IDLE;
    break;
  case TZ_UI_STREAM_CB_REFILL:
    break;
  case TZ_UI_STREAM_CB_REJECT:
    delay_reject();
    break;
  }
}

#ifdef HAVE_BAGL
__attribute__((noreturn)) static void prompt_address() {
  char buf[TZ_UI_STREAM_CONTENTS_SIZE + 1];

  FUNC_ENTER(("void"));
  tz_ui_stream_init(stream_cb);
  format_pkh(buf);
  tz_ui_stream_push_all(TZ_UI_STREAM_CB_NOCB, "Provide Key", buf,
                        TZ_UI_ICON_NONE);
  tz_ui_stream_push_accept_reject();
  tz_ui_stream_close();
  tz_ui_stream();
  FUNC_LEAVE();
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
  format_pkh(buf);

  nbgl_useCaseAddressConfirmation(buf, confirmation_callback);
}

__attribute__((noreturn)) static void prompt_address () {
  FUNC_ENTER(("void"));
  nbgl_useCaseReviewStart(&C_tezos, "Verify Tezos\naddress", NULL, "Cancel", verify_address, cancel_callback);
  THROW(ASYNC_EXCEPTION);
}
#endif

size_t handle_apdu_get_public_key(bool prompt) {
  uint8_t *dataBuffer = G_io_apdu_buffer + OFFSET_CDATA;

  FUNC_ENTER(("prompt=%s", prompt ? "true" : "false"));
  if (G_io_apdu_buffer[OFFSET_P1] != 0) THROW(EXC_WRONG_PARAM);

  // do not expose pks without prompt through U2F (permissionless legacy
  // comm in browser)
  if (!prompt) require_permissioned_comm();

  global.path_with_curve.derivation_type = G_io_apdu_buffer[OFFSET_CURVE];
  CX_THROW(check_derivation_type(global.path_with_curve.derivation_type));

  CX_THROW(read_bip32_path(&global.path_with_curve.bip32_path, dataBuffer,
                           G_io_apdu_buffer[OFFSET_LC]));

  if (!prompt) {
    return provide_pubkey(G_io_apdu_buffer);
  } else {
    global.step = ST_PROMPT;
    prompt_address();
  }
  FUNC_LEAVE();
}
