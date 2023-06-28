/* Tezos Ledger application - Some UI primitives

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

#include "ui_commons.h"
#include "utils.h"

#ifdef HAVE_BAGL
void io_seproxyhal_display(const bagl_element_t *element) {
    return io_seproxyhal_display_default((bagl_element_t *)element);
}

const bagl_icon_details_t C_icon_rien = { 0, 0, 1, NULL, NULL };
#endif

unsigned char io_event(__attribute__((unused)) unsigned char channel) {
  // nothing done with the event, throw an error on the transport layer if
  // needed

  // can't have more than one tag in the reply, not supported yet.
  switch (G_io_seproxyhal_spi_buffer[0]) {
#ifdef HAVE_NBGL
  case SEPROXYHAL_TAG_FINGER_EVENT:
    UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
    break;
#endif // HAVE_NBGL

  case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
#ifdef HAVE_BAGL
    UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
#endif // HAVE_BAGL
    break;
  case SEPROXYHAL_TAG_STATUS_EVENT:
    if (G_io_apdu_media == IO_APDU_MEDIA_USB_HID &&
        !(U4BE(G_io_seproxyhal_spi_buffer, 3) &
          SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
      THROW(EXCEPTION_IO_RESET);
    }
    // no break is intentional
    __attribute__((fallthrough));
  default:
    UX_DEFAULT_EVENT();
    break;

  case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
#ifdef HAVE_BAGL
    UX_DISPLAYED_EVENT({});
#endif // HAVE_BAGL
#ifdef HAVE_NBGL
    UX_DEFAULT_EVENT();
#endif // HAVE_NBGL
    break;

  case SEPROXYHAL_TAG_TICKER_EVENT:
   break;
  }

  // close the event if not done previously (by a display or whatever)
  if (!io_seproxyhal_spi_is_status_sent()) {
    io_seproxyhal_general_status();
  }

  // command has been processed, DO NOT reset the current APDU transport
  return 1;
}


// can't no return
void exit_app(void) {
  BEGIN_TRY_L(exit) {
    TRY_L(exit) {
      os_sched_exit(-1);
    }
    FINALLY_L(exit) {
    }
  }
  END_TRY_L(exit);
  THROW(0);  // Suppress warning
}
