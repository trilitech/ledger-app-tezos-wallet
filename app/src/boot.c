#include "os.h"
#include "cx.h"
#include "ui.h"

#include "globals.h"
#include "main_loop.h"

__attribute__((noreturn)) void main_loop();

__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    // ensure exception will work as planned
    os_boot();

    UX_INIT();

    uint8_t tag;
    init_globals();
    global.stack_root = &tag;

    for (;;) {
      BEGIN_TRY {
        TRY {
          io_seproxyhal_init();

#ifdef TARGET_NANOX
          // grab the current plane mode setting
          // requires "--appFlag 0x240" to be set in makefile
          G_io_app.plane_mode = os_setting_get(OS_SETTING_PLANEMODE, NULL, 0);
#endif  // TARGET_NANOX

          USB_power(0);
          USB_power(1);

#ifdef HAVE_BLE
          BLE_power(0, NULL);
          BLE_power(1, "Nano X");
#endif  // HAVE_BLE

          main_loop();
        }
        CATCH(EXCEPTION_IO_RESET) {
          // reset IO and UX
          continue;
        }
        CATCH_OTHER(e) {
          break;
        }
        FINALLY {
        }
      }
      END_TRY;
    }
    exit_app();
}
