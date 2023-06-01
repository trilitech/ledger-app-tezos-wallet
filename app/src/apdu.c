/* Tezos Ledger application - Some common primitives and some command handlers

   TODO: split this file (apdu primitives and apdu handlers)

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

#include "apdu.h"
#include "globals.h"

#include "apdu_sign.h"
#include "apdu_pubkey.h"

const uint8_t version[4] = {0 /* wallet */, APPVERSION_M, APPVERSION_N, APPVERSION_P};

size_t handle_apdu_version() {
    memcpy(G_io_apdu_buffer, &version, sizeof(version));
    size_t tx = sizeof(version);
    return finalize_successful_send(tx);
}

size_t handle_apdu_git() {
    static const char commit[] = COMMIT;
    memcpy(G_io_apdu_buffer, commit, sizeof(commit));
    size_t tx = sizeof(commit);
    return finalize_successful_send(tx);
}

size_t finalize_successful_send(size_t tx) {
  G_io_apdu_buffer[tx++] = 0x90;
  G_io_apdu_buffer[tx++] = 0x00;
  return tx;
}

// Send back response; do not restart the event loop
void delayed_send(size_t tx) {
  FUNC_ENTER(("tx=%u", tx));
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
  FUNC_LEAVE();
}

void delay_reject(void) {
  size_t tx = 0;
  FUNC_ENTER(("void"));
  G_io_apdu_buffer[tx++] = EXC_REJECT >> 8;
  G_io_apdu_buffer[tx++] = EXC_REJECT & 0xFF;
  delayed_send(tx);
  global.step = ST_IDLE;
  FUNC_LEAVE();
}

void require_permissioned_comm(void) {
  /* U2F is dangerous for privacy because any open website
     in the browser can use it silently if the app is opened.*/
  if (G_io_apdu_media == IO_APDU_MEDIA_U2F) {
    THROW(EXC_HID_REQUIRED);
  }
}
