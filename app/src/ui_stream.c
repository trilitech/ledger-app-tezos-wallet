/* Tezos Ledger application - Generic stream display

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 TriliTech <contact@trili.tech>

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
  size_t bucket, i;

  for (i = 0; i < TZ_SCREEN_LINES_11PX; i++)
    global.ux.lines[i][0]=0;

  if (global.stream.current == global.stream.total + 1) {
    STRLCPY(global.ux.lines[0], "Accept?");
    STRLCPY(global.ux.lines[1], "Press both buttons");
    STRLCPY(global.ux.lines[2], "to accept.");
    return;
  }

  if (global.stream.current == global.stream.total + 2) {
    STRLCPY(global.ux.lines[0], "Reject?");
    STRLCPY(global.ux.lines[1], "Press both buttons");
    STRLCPY(global.ux.lines[2], "to reject.");
    return;
  }

  bucket = global.stream.current % TZ_UI_STREAM_HISTORY_SCREENS;

  STRLCPY(global.ux.lines[0], global.stream.titles[bucket]);
  for (i = 0; i < TZ_UI_STREAM_CONTENTS_LINES; i++) {
    STRLCPY(global.ux.lines[i+1], global.stream.values[bucket] + i * TZ_UI_STREAM_CONTENTS_WIDTH);
  }
}

void tz_ui_stream_init (void (*cb)(tz_ui_cb_type_t)) {
  memset(&global.stream, 0x0, sizeof(global.stream));
  global.stream.cb = cb;
  global.stream.full = false;
  global.stream.current = -1;
  global.stream.total = -1;
}

void tz_ui_stream_close () {
  if (global.stream.full) {
    failwith("trying to close already closed stream display");
  }
  global.stream.full = true;
#ifdef TEZOS_DEBUG
  PRINTF("[DEBUG] close()\n");
#endif
}

size_t tz_ui_stream_push(const char *title, const char *value) {
  size_t i;

  if (global.stream.full) {
    failwith("trying to push in already closed stream display");
  }
#ifdef TEZOS_DEBUG
  int prev_total = global.stream.total;
  int prev_current = global.stream.current;
#endif

  global.stream.total++;
  int bucket = global.stream.total % TZ_UI_STREAM_HISTORY_SCREENS;
  STRLCPY(global.stream.titles[bucket], title);
  for (i = 0; i < TZ_UI_STREAM_CONTENTS_LINES; i++)
    global.stream.values[bucket][i * TZ_UI_STREAM_CONTENTS_WIDTH] = '\0';

  // Ensure things fit on one line
  int length = 0, offset = 0;
  while(value[length] != '\0') length++;

  int line = 0;
  while (offset < length && line < TZ_UI_STREAM_CONTENTS_LINES) {
    const char* start = value + offset;
    int len = length - offset;
    int will_fit = max_line_width(start, len, BAGL_FONT_OPEN_SANS_REGULAR_11px);

#ifdef TEZOS_DEBUG
  char debug_line[TZ_UI_STREAM_CONTENTS_SIZE + 1];
  debug_line[TZ_UI_STREAM_CONTENTS_SIZE] = 0;
  PRINTF("[DEBUG] split(value: \"%s\", will_fit: %d, length: %d, len: %d, line: %d, offset: %d)\n",
         value, will_fit, length, len, line, offset);
#endif

    char* buffer = global.stream.values[bucket] + line * TZ_UI_STREAM_CONTENTS_WIDTH;
    if (will_fit >= len && len < TZ_UI_STREAM_CONTENTS_WIDTH) {
      strlcpy(buffer, start, len + 1);
      offset += len;
      break;
    } else if (will_fit >= len && len > TZ_UI_STREAM_CONTENTS_WIDTH){
      strlcpy(buffer, start, TZ_UI_STREAM_CONTENTS_WIDTH + 1);
      offset += TZ_UI_STREAM_CONTENTS_WIDTH;
      line++;
      continue;
    } else if (will_fit <= TZ_UI_STREAM_CONTENTS_WIDTH) {
      strlcpy(buffer, start, will_fit + 1);
      offset += will_fit;
      line++;
      continue;
    } else {
      strlcpy(buffer, start, TZ_UI_STREAM_CONTENTS_WIDTH + 1);
      offset += TZ_UI_STREAM_CONTENTS_WIDTH;
      line++;
      continue;
    }
  }

  if (global.stream.total == 0 || global.stream.total >= TZ_UI_STREAM_HISTORY_SCREENS) {
    global.stream.current++;
  }

#ifdef TEZOS_DEBUG
  char debug_title[TZ_UI_STREAM_TITLE_WIDTH+1], debug_value[TZ_UI_STREAM_CONTENTS_SIZE + 1];
  debug_title[TZ_UI_STREAM_TITLE_WIDTH] = 0;
  debug_value[TZ_UI_STREAM_CONTENTS_SIZE] = 0;
  STRLCPY(debug_title, global.stream.titles[bucket]);
  STRLCPY(debug_value, global.stream.values[bucket]);
  PRINTF("[DEBUG] push_screen(title: \"%s\", value: \"%s\", total: %d -> %d, current: %d -> %d, offset: %d)\n",
         debug_title, debug_value,
         prev_total, global.stream.total, prev_current, global.stream.current, offset);
#endif

  return offset;
}

static void pred () {
  if (global.stream.current >= 1 && global.stream.current >= global.stream.total - TZ_UI_STREAM_HISTORY_SCREENS + 2) {
    global.stream.current--;
  }
}

static void succ () {
  if (global.stream.current < global.stream.total + (global.stream.full ? 2 : 0)) {
    global.stream.current++;
  }
}

tz_ui_stream_screen_kind tz_ui_stream_current_screen_kind () {
  if (global.stream.current < 0)
    return TZ_UI_STREAM_DISPLAY_INIT;
  else if (global.stream.current == 0)
    return TZ_UI_STREAM_DISPLAY_FIRST;
  else if (global.stream.current == global.stream.total - TZ_UI_STREAM_HISTORY_SCREENS + 1)
    return TZ_UI_STREAM_DISPLAY_CANNOT_GO_BACK;
  else if (global.stream.current == global.stream.total + 1)
    return TZ_UI_STREAM_DISPLAY_ACCEPT;
  else if (global.stream.current == global.stream.total + 2)
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
      global.stream.cb(TZ_UI_STREAM_CB_ACCEPT);
      break;
    case TZ_UI_STREAM_DISPLAY_REJECT:
      ui_initial_screen ();
      global.stream.cb(TZ_UI_STREAM_CB_REJECT);
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
    {{ BAGL_LABELINE, 0x02, 8, 8, 112, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BOLD, 0 }, global.ux.lines[0] },
    {{ BAGL_LABELINE, 0x02, 0, 19, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[1] },
    {{ BAGL_LABELINE, 0x02, 0, 30, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[2] },
    {{ BAGL_ICON, 0x00, 56, 14, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
#else
    {{ BAGL_RECTANGLE, 0x00, 0, 0, 128, BAGL_HEIGHT, 0, 0, BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL },
    {{ BAGL_ICON, 0x00, 1, 1, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_ICON, 0x00, 120, 1, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_LABELINE, 0x02, 8, 8, 112, 11, 0, 0, 0, 0xFFFFFF, 0x000000, BOLD, 0 }, global.ux.lines[0] },
    {{ BAGL_LABELINE, 0x02, 0, 21, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[1] },
    {{ BAGL_LABELINE, 0x02, 0, 34, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[2] },
    {{ BAGL_LABELINE, 0x02, 0, 47, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[3] },
    {{ BAGL_LABELINE, 0x02, 0, 60, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000, REGULAR, 0 }, global.ux.lines[4] },
    {{ BAGL_ICON, 0x00, 56, 47, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, 0, BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
#endif
  };

  update();

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
  if (global.stream.current == global.stream.total) {
    if (!global.stream.full)
      global.stream.cb(TZ_UI_STREAM_CB_REFILL);
  }
  // go back to the data screen
  succ ();
  redisplay();
}

__attribute__((noreturn)) void tz_ui_stream() {
  redisplay();
  THROW(ASYNC_EXCEPTION);
}
