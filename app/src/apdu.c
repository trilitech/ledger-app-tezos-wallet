#include "apdu.h"
#include "globals.h"
#include "version.h"

#include "apdu_sign.h"
#include "apdu_pubkey.h"

size_t handle_apdu_version() {
    memcpy(G_io_apdu_buffer, &version, sizeof(version_t));
    size_t tx = sizeof(version_t);
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
  io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx);
}

void delay_reject(void) {
  size_t tx = 0;
  G_io_apdu_buffer[tx++] = EXC_REJECT >> 8;
  G_io_apdu_buffer[tx++] = EXC_REJECT & 0xFF;
  delayed_send(tx);
  global.step = ST_IDLE;
}

void require_permissioned_comm(void) {
  /* U2F is dangerous for privacy because any open website
     in the browser can use it silently if the app is opened.*/
  if (G_io_apdu_media == IO_APDU_MEDIA_U2F) {
    THROW(EXC_HID_REQUIRED);
  }
}
