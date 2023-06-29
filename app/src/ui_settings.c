/*
 *
 */

#include "globals.h"

static void cb(tz_ui_cb_type_t);

#define BLIND_SIGNING	0x01
#define BACK    	0x02


static void
cb(tz_ui_cb_type_t type)
{
  FUNC_ENTER(("type=%u\n", type));

  switch (type) {
  case BLIND_SIGNING:
    global.settings.blindsigning = !global.settings.blindsigning;
    ui_settings_init();
    break;
  case BACK:
    ui_home_init();
    break;
  }
}

void
ui_settings_init(void)
{
  const char *bsigning = "DISABLED";

  FUNC_ENTER(("void"));

  if (global.settings.blindsigning)
    bsigning = "ENABLED";

  tz_ui_stream_init(cb);
  tz_ui_stream_push(BLIND_SIGNING, "Blind Signing", bsigning, TZ_UI_ICON_NONE);
  tz_ui_stream_push(BACK, "Back", "", TZ_UI_ICON_DASHBOARD);
  tz_ui_stream_close();
  tz_ui_stream_start();
  FUNC_LEAVE();
}
