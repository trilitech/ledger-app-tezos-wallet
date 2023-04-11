/* Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

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
#include "ui.h"

#include <string.h>

static size_t provide_pubkey(uint8_t *const io_buffer, cx_ecfp_public_key_t const *const pubkey) {
  check_null(io_buffer);
  check_null(pubkey);
  size_t tx = 0;
  // Application could be PIN-locked, and pubkey->W_len would then be 0,
  // so throwing an error rather than returning an empty key
  if (os_global_pin_is_validated() != BOLOS_UX_OK) {
    THROW(EXC_SECURITY);
  }
  io_buffer[tx++] = pubkey->W_len;
  memmove(io_buffer + tx, pubkey->W, pubkey->W_len);
  tx += pubkey->W_len;
  return finalize_successful_send(tx);
}

static void ok_cb() {
  cx_ecfp_public_key_t public_key = {0};
  generate_public_key(&public_key,
                      global.path_with_curve.derivation_type,
                      &global.path_with_curve.bip32_path);
  delayed_send(provide_pubkey(G_io_apdu_buffer, &public_key));
  global.step = ST_IDLE;
}

static void format_pkh(char* buffer) {
  cx_ecfp_public_key_t pubkey = {0};
  uint8_t hash[21];
  generate_public_key(&pubkey, global.path_with_curve.derivation_type, &global.path_with_curve.bip32_path);
  public_key_hash(hash+1, 20, NULL, global.path_with_curve.derivation_type, &pubkey);
  switch (global.path_with_curve.derivation_type) {
  case DERIVATION_TYPE_SECP256K1: hash[0] = 1; break;
  case DERIVATION_TYPE_SECP256R1: hash[0] = 2; break;
  case DERIVATION_TYPE_ED25519:
  case DERIVATION_TYPE_BIP32_ED25519: hash[0] = 0; break;
  default: THROW(EXC_WRONG_PARAM); // XXX find appropriate error
  }
  tz_format_pkh(hash, 21, buffer);
}


__attribute__((noreturn)) static void prompt_address () {
  tz_ui_stream_init(NULL, ok_cb, delay_reject);
  format_pkh(global.stream.buffer.value);
  strcpy(global.stream.buffer.title, "Provide Key");
  tz_ui_stream_push();
  tz_ui_stream_close();
  tz_ui_stream();
}

size_t handle_apdu_get_public_key(bool prompt) {
  uint8_t *dataBuffer = G_io_apdu_buffer + OFFSET_CDATA;

  if (G_io_apdu_buffer[OFFSET_P1] != 0) THROW(EXC_WRONG_PARAM);

  // do not expose pks without prompt through U2F (permissionless legacy comm in browser)
  if (!prompt) require_permissioned_comm();

  global.path_with_curve.derivation_type = parse_derivation_type(G_io_apdu_buffer[OFFSET_CURVE]);

  size_t const cdata_size = G_io_apdu_buffer[OFFSET_LC];

  read_bip32_path(&global.path_with_curve.bip32_path, dataBuffer, cdata_size);

  cx_ecfp_public_key_t public_key = {0};
  generate_public_key(&public_key,
                      global.path_with_curve.derivation_type,
                      &global.path_with_curve.bip32_path);

  if (!prompt) {
    return provide_pubkey(G_io_apdu_buffer, &public_key);
  } else {
    global.step = ST_PROMPT;
    prompt_address();
  }
}
