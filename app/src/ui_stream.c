/* Tezos Ledger application - Generic stream display

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include "globals.h"

// Model

static void update () {
  if (global.stream.model.current == global.stream.model.total + 1) {
    strcpy(global.stream.model.title, "Accept?");
    bzero(global.stream.model.value, TZ_UI_STREAM_CONTENTS_SIZE);
    strcpy(global.stream.model.value, "Press both buttons");
    strcpy(global.stream.model.value + TZ_UI_STREAM_CONTENTS_WIDTH, "to accept.");
  } else if (global.stream.model.current == global.stream.model.total + 2) {
    strcpy(global.stream.model.title, "Reject?");
    bzero(global.stream.model.value, TZ_UI_STREAM_CONTENTS_SIZE);
    strcpy(global.stream.model.value, "Press both buttons");
    strcpy(global.stream.model.value + TZ_UI_STREAM_CONTENTS_WIDTH, "to reject.");
  } else if (global.stream.model.current >= 0) {
    int bucket = global.stream.model.current % TZ_UI_STREAM_HISTORY_SCREENS;
    memcpy(global.stream.model.title, global.stream.model.titles[bucket], TZ_UI_STREAM_TITLE_WIDTH);
    memcpy(global.stream.model.value, global.stream.model.values[bucket], TZ_UI_STREAM_CONTENTS_SIZE);
  }
#ifdef TEZOS_DEBUG
  PRINTF("[DEBUG] update_screen(title: \"%s\", value: \"%s\")\n",
         global.stream.model.title, global.stream.model.value);
#endif
}

void tz_ui_stream_init (void (*refill)(), void (*accept)(), void (*reject)()) {
  global.stream.callbacks.refill = refill;
  global.stream.callbacks.accept = accept;
  global.stream.callbacks.reject = reject;
  global.stream.model.full = false;
  global.stream.model.current = -1;
  global.stream.model.total = -1;
  global.stream.model.title[TZ_UI_STREAM_TITLE_WIDTH] = 0;
  global.stream.model.value[TZ_UI_STREAM_CONTENTS_SIZE] = 0;
  strncpy(global.stream.model.title, "Waiting", TZ_UI_STREAM_TITLE_WIDTH);
  strncpy(global.stream.model.value, "", TZ_UI_STREAM_CONTENTS_SIZE);
}

void tz_ui_stream_close () {
  if (global.stream.model.full) {
    failwith("trying to close already closed stream display");
  }
  global.stream.model.full = true;
#ifdef TEZOS_DEBUG
  PRINTF("[DEBUG] close()\n");
#endif
}

void tz_ui_stream_push () {
  if (global.stream.model.full) {
    failwith("trying to push in already closed stream display");
  }
#ifdef TEZOS_DEBUG
  int prev_total = global.stream.model.total;
  int prev_current = global.stream.model.current;
#endif

  global.stream.model.total++;
  int bucket = global.stream.model.total % TZ_UI_STREAM_HISTORY_SCREENS;
  strncpy(global.stream.model.titles[bucket], global.stream.buffer.title, TZ_UI_STREAM_TITLE_WIDTH);
  strncpy(global.stream.model.values[bucket], global.stream.buffer.value, TZ_UI_STREAM_CONTENTS_SIZE);
  if (global.stream.model.total == 0 || global.stream.model.total >= TZ_UI_STREAM_HISTORY_SCREENS) {
    global.stream.model.current++;
    update ();
  }
#ifdef TEZOS_DEBUG
  char debug_title[TZ_UI_STREAM_TITLE_WIDTH+1], debug_value[TZ_UI_STREAM_CONTENTS_SIZE + 1];
  debug_title[TZ_UI_STREAM_TITLE_WIDTH] = 0;
  debug_value[TZ_UI_STREAM_CONTENTS_SIZE] = 0;
  strncpy(debug_title, global.stream.model.titles[bucket], TZ_UI_STREAM_TITLE_WIDTH);
  strncpy(debug_value, global.stream.model.values[bucket], TZ_UI_STREAM_CONTENTS_SIZE);
  PRINTF("[DEBUG] push_screen(title: \"%s\", value: \"%s\", total: %d -> %d, current: %d -> %d)\n",
         debug_title, debug_value,
         prev_total, global.stream.model.total, prev_current, global.stream.model.current);
#endif
}

static void pred () {
  if (global.stream.model.current >= 1 && global.stream.model.current >= global.stream.model.total - TZ_UI_STREAM_HISTORY_SCREENS + 2) {
    global.stream.model.current--;
    update ();
  }
}

static void succ () {
  if (global.stream.model.current < global.stream.model.total + (global.stream.model.full ? 2 : 0)) {
    global.stream.model.current++;
    update ();
  }
}

tz_ui_stream_screen_kind tz_ui_stream_current_screen_kind () {
  if (global.stream.model.current < 0)
    return TZ_UI_STREAM_DISPLAY_INIT;
  else if (global.stream.model.current == 0)
    return TZ_UI_STREAM_DISPLAY_FIRST;
  else if (global.stream.model.current == global.stream.model.total - TZ_UI_STREAM_HISTORY_SCREENS + 1)
    return TZ_UI_STREAM_DISPLAY_CANNOT_GO_BACK;
  else if (global.stream.model.current == global.stream.model.total + 1)
    return TZ_UI_STREAM_DISPLAY_ACCEPT;
  else if (global.stream.model.current == global.stream.model.total + 2)
    return TZ_UI_STREAM_DISPLAY_REJECT;
  else
    return TZ_UI_STREAM_DISPLAY_CONT;
}

// View

static void change_screen_left();
static void change_screen_right();

static unsigned int cb(unsigned int button_mask, __attribute__((unused)) unsigned int button_mask_counter) {
  switch (button_mask) {
  case BUTTON_EVT_RELEASED | BUTTON_LEFT:
    change_screen_left();
    break;
  case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
    change_screen_right();
    break;
  case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
    switch (tz_ui_stream_current_screen_kind ()) {
    case TZ_UI_STREAM_DISPLAY_ACCEPT:
      ui_initial_screen ();
      global.stream.callbacks.accept ();
      break;
    case TZ_UI_STREAM_DISPLAY_REJECT:
      ui_initial_screen ();
      global.stream.callbacks.reject ();
      break;
    default: break;
    }
    break;
  default:
    break;
  }
  return 0;
}

static void redisplay () {
  bagl_element_t init[4 + TZ_SCREEN_LINES_11PX] = {
    // { {type, userid, x, y, width, height, stroke, radius, fill, fgcolor, bgcolor, font_id, icon_id}, text/icon
#ifdef TARGET_NANOS
    {{ BAGL_RECTANGLE, 0x00, 0, 0, 128, BAGL_HEIGHT, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL },
    {{ BAGL_ICON, 0x00, 1, 1, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_ICON, 0x00, 120, 1, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_LABELINE, 0x02, 8, 8, 112, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BOLD, 0 }, global.stream.model.title },
    {{ BAGL_LABELINE, 0x02, 0, 19, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[1] },
    {{ BAGL_LABELINE, 0x02, 0, 30, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[2] },
    {{ BAGL_ICON, 0x00, 56, 14, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
#else
    {{ BAGL_RECTANGLE, 0x00, 0, 0, 128, BAGL_HEIGHT, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL },
    {{ BAGL_ICON, 0x00, 1, 1, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_ICON, 0x00, 120, 1, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_LABELINE, 0x02, 8, 8, 112, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BOLD, 0 }, global.stream.model.title },
    {{ BAGL_LABELINE, 0x02, 0, 21, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[1] },
    {{ BAGL_LABELINE, 0x02, 0, 34, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[2] },
    {{ BAGL_LABELINE, 0x02, 0, 47, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[3] },
    {{ BAGL_LABELINE, 0x02, 0, 60, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[4] },
    {{ BAGL_ICON, 0x00, 56, 47, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
#endif
  };

  for (int i = 0; i < TZ_SCREEN_LINES_11PX;i++)
    global.ux.lines[i][0]=0;
  for (int i = 0; i < TZ_UI_STREAM_CONTENTS_LINES;i++) {
    strncpy(global.ux.lines[i+1], global.stream.model.value + i * TZ_UI_STREAM_CONTENTS_WIDTH, TZ_UI_STREAM_CONTENTS_WIDTH);
  }

  switch (tz_ui_stream_current_screen_kind ()) {
  case TZ_UI_STREAM_DISPLAY_INIT:
  case TZ_UI_STREAM_DISPLAY_FIRST:
    init[2].text = (const char*) &C_icon_go_right;
    break;
  case TZ_UI_STREAM_DISPLAY_CANNOT_GO_BACK:
    init[1].text = (const char*) &C_icon_go_forbid;
    init[2].text = (const char*) &C_icon_go_right;
    break;
  case TZ_UI_STREAM_DISPLAY_CONT:
    init[1].text = (const char*) &C_icon_go_left;
    init[2].text = (const char*) &C_icon_go_right;
    break;
  case TZ_UI_STREAM_DISPLAY_ACCEPT:
    init[1].text = (const char*) &C_icon_go_left;
    init[2].text = (const char*) &C_icon_go_right;
#ifdef TARGET_NANOS
    global.ux.lines[1][0] = 0;
    global.ux.lines[2][0] = 0;
#endif
    init[sizeof(init)/sizeof(bagl_element_t)-1].text = (const char*) &C_icon_validate_14;
    break;
  case TZ_UI_STREAM_DISPLAY_REJECT:
    init[1].text = (const char*) &C_icon_go_left;
    init[sizeof(init)/sizeof(bagl_element_t)-1].text = (const char*) &C_icon_crossmark;
#ifdef TARGET_NANOS
    global.ux.lines[1][0] = 0;
    global.ux.lines[2][0] = 0;
#endif
    break;
  }
  DISPLAY(init, cb);
}

static void change_screen_left() {
  pred ();
  redisplay();
}

static void change_screen_right() {
  if (global.stream.model.current == global.stream.model.total) {
    if (!global.stream.model.full) {
      global.stream.callbacks.refill ();
    }
  }
  // go back to the data screen
  succ ();
  redisplay();
}

__attribute__((noreturn)) void tz_ui_stream() {
  redisplay();
  THROW(ASYNC_EXCEPTION);
}
