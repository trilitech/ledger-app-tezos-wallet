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

#ifdef HAVE_BAGL
static unsigned int cb(unsigned int, unsigned int);
static const char *find_icon(tz_ui_icon_t);
static void pred(void);
static void succ(void);
static void change_screen_left(void);
static void change_screen_right(void);
static void redisplay(void);

const bagl_icon_details_t C_icon_rien = { 0, 0, 1, NULL, NULL };
#endif // HAVE_BAGL

// Model

void tz_ui_stream_init(void (*cb)(uint8_t)) {
  tz_ui_stream_t *s = &global.stream;

  FUNC_ENTER(("cb=%p", cb));
  memset(s, 0x0, sizeof(*s));
  s->cb = cb;
  s->full = false;
  s->current = 0;
  s->total = -1;
  FUNC_LEAVE();
}

void tz_ui_stream_push_accept_reject(void) {
  FUNC_ENTER(("void"));
  tz_ui_stream_push(TZ_UI_STREAM_CB_ACCEPT, "Accept?",
                    "Press both buttons to accept.", TZ_UI_ICON_TICK);
  tz_ui_stream_push(TZ_UI_STREAM_CB_REJECT, "Reject?",
                    "Press both buttons to reject.", TZ_UI_ICON_CROSS);
  FUNC_LEAVE();
}

void tz_ui_stream_close() {
  tz_ui_stream_t *s = &global.stream;

  FUNC_ENTER(("void"));
  if (s->full)
    failwith("trying to close already closed stream display");
  s->full = true;
  FUNC_LEAVE();
}

#ifdef HAVE_BAGL
uint8_t tz_ui_max_line_chars(const char* value, int length) {
  uint8_t will_fit = MIN(TZ_UI_STREAM_CONTENTS_WIDTH, length);

  FUNC_ENTER(("value=\"%s\", length=%d", value, length));

  /* Wrap on newline */
  const char *tmp = memchr(value, '\n', will_fit);
  if (tmp && (tmp - value) <= will_fit)
    will_fit = (tmp - value);

# ifdef TARGET_NANOS
    will_fit = se_get_cropped_length(value, will_fit, BAGL_WIDTH,
                                     BAGL_ENCODING_LATIN1);
# else
    uint8_t width;
    will_fit++;
    do {
      width = bagl_compute_line_width(BAGL_FONT_OPEN_SANS_REGULAR_11px, 0,
                                      value, --will_fit, BAGL_ENCODING_LATIN1);
    } while (width >= BAGL_WIDTH);

    PRINTF("[DEBUG] max_line_width(value: \"%s\", width: %d, will_fit: %d)\n",
            value, width, will_fit);
# endif

  FUNC_LEAVE();
  return will_fit;
}

size_t tz_ui_stream_push_all(tz_ui_cb_type_t type, const char *title,
                             const char *value, tz_ui_icon_t icon) {
  size_t obuflen;
  size_t i = 0;

  obuflen = strlen(value);
  do {
    i += tz_ui_stream_push(type, title, value + i, icon);
  } while (i < obuflen);

  return i;
}

size_t tz_ui_stream_push(tz_ui_cb_type_t type, const char *title,
                         const char *value, tz_ui_icon_t icon) {
  return tz_ui_stream_pushl(type, title, value, -1, icon);
}

size_t tz_ui_stream_pushl(tz_ui_cb_type_t type, const char *title,
                          const char *value, ssize_t max, tz_ui_icon_t icon) {
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

  STRLCPY(s->screens[bucket].title, title);
  for (i = 0; i < TZ_UI_STREAM_CONTENTS_LINES; i++)
    s->screens[bucket].body[i][0] = '\0';

  // Ensure things fit on one line
  size_t length = strlen(value);
  size_t offset = 0;

  if (max != -1)
    length = MIN(length, (size_t)max);

  s->screens[bucket].type = type;
  s->screens[bucket].icon = icon;

  int line = 0;
  while (offset < length && line < TZ_UI_STREAM_CONTENTS_LINES) {
    uint8_t will_fit;

    if (value[offset] == '\n')
      offset++;

    will_fit = tz_ui_max_line_chars(&value[offset], length - offset);

    PRINTF("[DEBUG] split(value: \"%s\", will_fit: %d, line: %d, "
           "offset: %d)\n", &value[offset], will_fit, line, offset);

    strlcpy(s->screens[bucket].body[line], &value[offset], will_fit + 1);

    offset += will_fit;

    line++;
  }

  PRINTF("[DEBUG] tz_ui_stream_pushl(%s, %s, %u)\n", title, value, max);
  PRINTF("[DEBUG]        bucket     %d\n", bucket);
  PRINTF("[DEBUG]        title:     \"%s\"\n", s->screens[bucket].title);
  for (line=0; line < TZ_UI_STREAM_CONTENTS_LINES; line++)
    PRINTF("[DEBUG]        value[%d]: \"%s\"\n", line,
           s->screens[bucket].body[line]);
  PRINTF("[DEBUG]        total:     %d -> %d\n", prev_total, s->total);
  PRINTF("[DEBUG]        current:   %d -> %d\n", prev_current, s->current);
  PRINTF("[DEBUG]        offset:    %d\n", offset);
  FUNC_LEAVE();

  return offset;
}
#endif // HAVE_BAGL

tz_ui_cb_type_t tz_ui_stream_get_type(void) {
  tz_ui_stream_t *s = &global.stream;
  size_t bucket = s->current % TZ_UI_STREAM_HISTORY_SCREENS;

  return s->screens[bucket].type;
}

#ifdef HAVE_BAGL
static void pred() {
  tz_ui_stream_t *s = &global.stream;

  FUNC_ENTER(("void"));
  if (s->current >= 1 &&
      s->current >= s->total - TZ_UI_STREAM_HISTORY_SCREENS + 2) {
    s->current--;
  }
  FUNC_LEAVE();
}

static void succ() {
  tz_ui_stream_t *s = &global.stream;

  FUNC_ENTER(("void"));
  if (s->current < s->total) {
    s->pressed_right = false;
    s->current++;
  }
  FUNC_LEAVE();
}
#endif // HAVE_BAGL

// View

#ifdef HAVE_BAGL
static unsigned int cb(unsigned int button_mask,
                       __attribute__((unused))
                       unsigned int button_mask_counter) {
  tz_ui_stream_t *s = &global.stream;
  size_t bucket = s->current % TZ_UI_STREAM_HISTORY_SCREENS;
  uint8_t type = s->screens[bucket].type;

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
    if (type)
      s->cb(type);
    if (type & TZ_UI_STREAM_CB_MAINMASK)
      ui_home_init();
    break;
  default:
    break;
  }
  FUNC_LEAVE();
  return 0;
}

static const char *find_icon(tz_ui_icon_t icon) {

  switch (icon) {
  case TZ_UI_ICON_TICK:       return (const char *)&C_icon_validate_14;
  case TZ_UI_ICON_CROSS:      return (const char *)&C_icon_crossmark;
  case TZ_UI_ICON_DASHBOARD:  return (const char *)&C_icon_dashboard_x;
  case TZ_UI_ICON_SETTINGS:   return (const char *)&C_icon_settings;
  default:                    return NULL;
  }
}

static void redisplay() {
  bagl_element_t init[] = {
    //  {type, userid, x, y, width, height, stroke, radius,
    //   fill, fgcolor, bgcolor, font_id, icon_id}, text/icon
    {{ BAGL_RECTANGLE, 0x00, 0, 0, 128, BAGL_HEIGHT, 0, 0,
       BAGL_FILL, 0x000000, 0xFFFFFF, 0, 0 }, NULL },
    {{ BAGL_ICON, 0x00, 1, 1, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
       BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_ICON, 0x00, 120, 1, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
       BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
    {{ BAGL_LABELINE, 0x02, 8, 8, 112, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
       BOLD, 0 }, global.ux.lines[0] },
#ifdef TARGET_NANOS
    {{ BAGL_LABELINE, 0x02, 0, 19, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
       REGULAR, 0 }, global.ux.lines[1] },
    {{ BAGL_ICON, 0x00, 56, 14, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
       BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
#else
    {{ BAGL_LABELINE, 0x02, 0, 21, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
       REGULAR, 0 }, global.ux.lines[1] },
    {{ BAGL_LABELINE, 0x02, 0, 34, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
       REGULAR, 0 }, global.ux.lines[2] },
    {{ BAGL_LABELINE, 0x02, 0, 47, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
       REGULAR, 0 }, global.ux.lines[3] },
    {{ BAGL_LABELINE, 0x02, 0, 60, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
       REGULAR, 0 }, global.ux.lines[4] },
    {{ BAGL_ICON, 0x00, 56, 47, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
       BAGL_GLYPH_NOGLYPH }, (const char*) &C_icon_rien },
#endif
  };

  tz_ui_stream_t *s = &global.stream;
  size_t bucket, i;

  FUNC_ENTER(("void"));

  for (i = 0; i < TZ_SCREEN_LINES_11PX; i++)
    global.ux.lines[i][0]=0;

  bucket = s->current % TZ_UI_STREAM_HISTORY_SCREENS;

  STRLCPY(global.ux.lines[0], s->screens[bucket].title);
  for (i = 0; i < TZ_UI_STREAM_CONTENTS_LINES; i++) {
    STRLCPY(global.ux.lines[i+1], s->screens[bucket].body[i]);
  }

  tz_ui_icon_t icon = s->screens[bucket].icon;
  if (icon) {
#ifdef TARGET_NANOS
    global.ux.lines[1][0] = 0;
#endif
    init[sizeof(init)/sizeof(bagl_element_t)-1].text = find_icon(icon);
  }

  /* If we aren't on the first screen, we can go back */
  if (s->current > 0)
    init[1].text = (const char*) &C_icon_go_left;

  /* Unless we can't... */
  if (s->current == s->total - TZ_UI_STREAM_HISTORY_SCREENS + 1)
    init[1].text = (const char*) &C_icon_go_forbid;

  /* If we aren't full or aren't on the last page, we can go right */
  if (!s->full || s->current < s->total)
    init[2].text = (const char*) &C_icon_go_right;
    
  DISPLAY(init, cb);
  FUNC_LEAVE();
}

static void change_screen_left() {
  FUNC_ENTER(("void"));
  pred();
  redisplay();
  FUNC_LEAVE();
}

static void change_screen_right() {
  tz_ui_stream_t *s = &global.stream;

  FUNC_ENTER(("void"));
  s->pressed_right = true;
  if (s->current == s->total) {
    if (!s->full)
      s->cb(TZ_UI_STREAM_CB_REFILL);
  }
  // go back to the data screen
  succ();
  redisplay();
  FUNC_LEAVE();
}
#endif // HAVE_BAGL

void tz_ui_stream_start(void) {
  FUNC_ENTER(("void"));
#ifdef HAVE_BAGL
  redisplay();
#endif
  FUNC_LEAVE();
}

void tz_ui_stream() {
  FUNC_ENTER(("void"));

#ifdef HAVE_BAGL
  tz_ui_stream_t *s = &global.stream;
  if (s->pressed_right)
    succ();

  redisplay();
#endif // HAVE_BAGL
  FUNC_LEAVE();
}
