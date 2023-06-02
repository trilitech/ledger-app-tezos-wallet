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

/* Prototypes */

static unsigned int cb(unsigned int, unsigned int);
static void update(void);
static void pred(void);
static void succ(void);
static void change_screen_left(void);
static void change_screen_right(void);
static void redisplay(void);


// Model

static void update () {
  size_t bucket, i;

  FUNC_ENTER(("void"));

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
  FUNC_LEAVE();
}

void tz_ui_stream_init (void (*cb)(tz_ui_cb_type_t)) {
  FUNC_ENTER(("cb=%p", cb));
  memset(&global.stream, 0x0, sizeof(global.stream));
  global.stream.cb = cb;
  global.stream.full = false;
  global.stream.current = -1;
  global.stream.total = -1;
  FUNC_LEAVE();
}

void tz_ui_stream_close () {
  FUNC_ENTER(("void"));
  if (global.stream.full) {
    failwith("trying to close already closed stream display");
  }
  global.stream.full = true;
  FUNC_LEAVE();
}

uint8_t tz_ui_max_line_chars(const char* value, int length) {
  if (length > TZ_UI_STREAM_CONTENTS_WIDTH) {
    length = TZ_UI_STREAM_CONTENTS_WIDTH;
  }

# ifdef TARGET_NANOS
    uint8_t will_fit = se_get_cropped_length(value, length, BAGL_WIDTH, BAGL_ENCODING_LATIN1);
# else
    uint8_t will_fit = length + 1;
    uint8_t width;
    do {
      will_fit--;
      width = bagl_compute_line_width(BAGL_FONT_OPEN_SANS_REGULAR_11px, 0, value, will_fit, BAGL_ENCODING_LATIN1);
    } while (width >= BAGL_WIDTH);

    PRINTF("[DEBUG] max_line_width(value: \"%s\", width: %d, will_fit: %d)\n",
            value, width, will_fit);
# endif

    return will_fit;
}

size_t tz_ui_stream_push(const char *title, const char *value) {
  return tz_ui_stream_pushl(title, value, sizeof(global.stream.values[0]));
}

size_t tz_ui_stream_pushl(const char *title, const char *value, size_t max) {
  tz_ui_stream_t *s = &global.stream;
  size_t i;

  FUNC_ENTER(("title=%s, value=%s", title, value));
  if (s->full) {
    failwith("trying to push in already closed stream display");
  }
#ifdef TEZOS_DEBUG
  int prev_total = s->total;
  int prev_current = s->current;
#endif

  s->total++;
  int bucket = s->total % TZ_UI_STREAM_HISTORY_SCREENS;

  STRLCPY(s->titles[bucket], title);
  for (i = 0; i < TZ_UI_STREAM_CONTENTS_LINES; i++)
    s->values[bucket][i * TZ_UI_STREAM_CONTENTS_WIDTH] = '\0';

  // Ensure things fit on one line
  size_t length = MIN(strlen(value), max);
  size_t offset = 0;

  int line = 0;
  while (offset < length && line < TZ_UI_STREAM_CONTENTS_LINES) {
    const char* start = value + offset;
    int len = length - offset;
    int will_fit = tz_ui_max_line_chars(start, len);

    PRINTF("[DEBUG] split(value: \"%s\", will_fit: %d, len: %d, line: %d, offset: %d)\n",
            start, will_fit, len, line, offset);

    char* buffer = s->values[bucket] + line * TZ_UI_STREAM_CONTENTS_WIDTH;
    strlcpy(buffer, start, will_fit + 1);
    offset += will_fit;

    line++;
  }

  if (s->total == 0 || s->total >= TZ_UI_STREAM_HISTORY_SCREENS) 
    s->current++;

  PRINTF("[DEBUG] tz_ui_stream_pushl(%s, %s, %u)\n", title, value, max);
  PRINTF("[DEBUG]        bucket   %d\n", bucket);
  PRINTF("[DEBUG]        title:   \"%s\"\n", s->titles[bucket]);
  PRINTF("[DEBUG]        value:   \"%s\"\n", s->values[bucket]);
  PRINTF("[DEBUG]        total:   %d -> %d\n", prev_total, s->total);
  PRINTF("[DEBUG]        current: %d -> %d\n", prev_current, s->current);
  PRINTF("[DEBUG]        offset:  %d\n", offset);
  FUNC_LEAVE();

  return offset;
}

static void pred () {
  FUNC_ENTER(("void"));
  if (global.stream.current >= 1 && global.stream.current >= global.stream.total - TZ_UI_STREAM_HISTORY_SCREENS + 2) {
    global.stream.current--;
  }
  FUNC_LEAVE();
}

static void succ () {
  FUNC_ENTER(("void"));
  if (global.stream.current < global.stream.total + (global.stream.full ? 2 : 0)) {
    global.stream.pressed_right = false;
    global.stream.current++;
  }
  FUNC_LEAVE();
}

tz_ui_stream_screen_kind tz_ui_stream_current_screen_kind () {
  FUNC_ENTER(("void"));
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
  FUNC_LEAVE();
}

// View

static unsigned int cb(unsigned int button_mask, __attribute__((unused)) unsigned int button_mask_counter) {

  FUNC_ENTER(("button_mask=%d, button_mask_counter=%d", button_mask,
              button_mask_counter));

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
  FUNC_LEAVE();
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

  FUNC_ENTER(("void"));

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
  FUNC_LEAVE();
}

static void change_screen_left() {
  FUNC_ENTER(("void"));
  pred ();
  redisplay();
  FUNC_LEAVE();
}

static void change_screen_right() {
  global.stream.pressed_right = true;

  FUNC_ENTER(("void"));
  if (global.stream.current == global.stream.total) {
    if (!global.stream.full)
      global.stream.cb(TZ_UI_STREAM_CB_REFILL);
  }
  // go back to the data screen
  succ ();
  redisplay();
  FUNC_LEAVE();
}

__attribute__((noreturn)) void tz_ui_stream() {
  FUNC_ENTER(("void"));
  if (global.stream.pressed_right)
    succ();

  redisplay();
  THROW(ASYNC_EXCEPTION);
}
