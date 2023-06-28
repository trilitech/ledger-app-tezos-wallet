/* Tezos Ledger application - Home screen display

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 TriliTech <contact@trili.tech>

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

void tz_ui_home_redisplay(void);
unsigned int cb(unsigned int, unsigned int);

unsigned int cb(unsigned int button_mask, __attribute__((unused)) unsigned int button_mask_counter) {
  FUNC_ENTER(("button_mask=%d, button_mask_counter=%d", button_mask,
              button_mask_counter));

  if ((button_mask & BUTTON_EVT_RELEASED) == 0)
    return 0;

  switch (button_mask & ~BUTTON_EVT_RELEASED) {
  case BUTTON_LEFT:
    if (global.home_screen > SCREEN_CLEAR_SIGN) {
      global.home_screen--;
      tz_ui_home_redisplay();
    }
    break;
  case BUTTON_RIGHT:
    if (global.home_screen < SCREEN_QUIT) {
      global.home_screen++;
      tz_ui_home_redisplay();
    }
    break;
  case BUTTON_LEFT | BUTTON_RIGHT:
    if (global.home_screen == SCREEN_QUIT)
      exit_app();
    break;
  }

  FUNC_LEAVE();
  return 0;
}

void tz_ui_home_redisplay(void) {
  bagl_element_t init[4 + TZ_SCREEN_LINES_11PX] = {
    // { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id}, text/icon
#ifdef TARGET_NANOS
    {{ BAGL_RECTANGLE, 0x00, 0, 0, 128, BAGL_HEIGHT, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL },
    {{ BAGL_ICON, 0x00, 3, 1, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_ICON, 0x00, 122, 1, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_LABELINE, 0x02, 7, 8, 114, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BOLD, 0 }, global.ux.lines[0] },
    {{ BAGL_LABELINE, 0x02, 0, 19, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[1] },
    {{ BAGL_LABELINE, 0x02, 0, 30, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[2] },
    {{ BAGL_ICON, 0x00, 56, 14, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
#define LINE_1 1
#define LINE_2 2
#else
    {{ BAGL_RECTANGLE, 0x00, 0, 0, 128, BAGL_HEIGHT, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL },
    {{ BAGL_ICON, 0x00, 3, 1, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_ICON, 0x00, 122, 1, 4, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_LABELINE, 0x02, 7, 8, 114, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BOLD, 0 }, global.ux.lines[0] },
    {{ BAGL_LABELINE, 0x02, 0, 21, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[1] },
    {{ BAGL_LABELINE, 0x02, 0, 34, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[2] },
    {{ BAGL_LABELINE, 0x02, 0, 47, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[3] },
    {{ BAGL_LABELINE, 0x02, 0, 60, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[4] },
    {{ BAGL_ICON, 0x00, 56, 24, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
#define LINE_1 2
#define LINE_2 3
#endif
  };

  FUNC_ENTER(("void"));

  for (int i = 0; i < TZ_SCREEN_LINES_11PX;i++)
    global.ux.lines[i][0]=0;
  switch (global.home_screen) {
  case SCREEN_CLEAR_SIGN:
    init[2].text = (const char*) &C_icon_right;
    STRLCPY(global.ux.lines[0], "Tezos Wallet");
    STRLCPY(global.ux.lines[LINE_1], "ready for");
    STRLCPY(global.ux.lines[LINE_2], "safe signing");
    break;
  case SCREEN_BLIND_SIGN:
    init[1].text = (const char*) &C_icon_left;
    init[2].text = (const char*) &C_icon_right;
    STRLCPY(global.ux.lines[0], "Tezos Wallet");
    STRLCPY(global.ux.lines[LINE_1], "ready for");
    STRLCPY(global.ux.lines[LINE_2], "BLIND signing");
    break;
  case SCREEN_QUIT:
    init[1].text = (const char*) &C_icon_left;
    STRLCPY(global.ux.lines[0], "Quit?");
    init[sizeof(init)/sizeof(bagl_element_t)-1].text = (const char*) &C_icon_dashboard_x;
    break;
  case SCREEN_SETTINGS:
    init[1].text = (const char*) &C_icon_left;
    init[2].text = (const char*) &C_icon_right;
    STRLCPY(global.ux.lines[0], "Settings");
    init[sizeof(init)/sizeof(bagl_element_t)-1].text = (const char*) &C_icon_settings;
    break;
  }
  DISPLAY(init, cb);
  FUNC_LEAVE();
}

#endif // HAVE_BAGL
