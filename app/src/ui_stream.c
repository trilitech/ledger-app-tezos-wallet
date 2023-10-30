/* Tezos Ledger application - Generic stream display

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>
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
static const char  *find_icon(tz_ui_icon_t);
static void         pred(void);
static void         succ(void);
static void         change_screen_left(void);
static void         change_screen_right(void);
static void         redisplay(void);

const bagl_icon_details_t C_icon_rien = {0, 0, 1, NULL, NULL};
#endif  // HAVE_BAGL

// Model

#ifdef HAVE_BAGL
void
tz_ui_stream_init(void (*cb)(uint8_t))
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("cb=%p", cb));
    memset(s, 0x0, sizeof(*s));
    s->cb      = cb;
    s->full    = false;
    s->current = 0;
    s->total   = -1;
    FUNC_LEAVE();
}
#endif

void
tz_ui_stream_push_accept_reject(void)
{
    FUNC_ENTER(("void"));
#ifdef HAVE_BAGL
    tz_ui_stream_push(TZ_UI_STREAM_CB_ACCEPT, "Accept?",
                      "Press both buttons to accept.", TZ_UI_ICON_TICK);
    tz_ui_stream_push(TZ_UI_STREAM_CB_REJECT, "Reject?",
                      "Press both buttons to reject.", TZ_UI_ICON_CROSS);
#endif
    FUNC_LEAVE();
}

#ifdef HAVE_BAGL
void
tz_ui_stream_close(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));
    if (s->full) {
        PRINTF("trying to close already closed stream display");
        THROW(EXC_UNKNOWN);
    }
    s->full = true;
    FUNC_LEAVE();
}
#endif  // HAVE_BAGL

uint8_t
tz_ui_max_line_chars(const char *value, int length)
{
    uint8_t will_fit = MIN(TZ_UI_STREAM_CONTENTS_WIDTH, length);

    FUNC_ENTER(("value=\"%s\", length=%d", value, length));

    /* Wrap on newline */
    const char *tmp = memchr(value, '\n', will_fit);
    if (tmp && (tmp - value) <= will_fit)
        will_fit = (tmp - value);

#ifdef TARGET_NANOS
    will_fit = se_get_cropped_length(value, will_fit, BAGL_WIDTH,
                                     BAGL_ENCODING_LATIN1);
#elif defined(HAVE_BAGL)
    uint8_t width;
    will_fit++;
    do {
        width = bagl_compute_line_width(BAGL_FONT_OPEN_SANS_REGULAR_11px, 0,
                                        value, --will_fit,
                                        BAGL_ENCODING_LATIN1);
    } while (width >= BAGL_WIDTH);

    PRINTF("[DEBUG] max_line_width(value: \"%s\", width: %d, will_fit: %d)\n",
           value, width, will_fit);
#endif

    FUNC_LEAVE();
    return will_fit;
}

size_t
tz_ui_stream_push_all(tz_ui_cb_type_t type, const char *title,
                      const char *value, tz_ui_icon_t icon)
{
    size_t obuflen;
    size_t i = 0;

    obuflen = strlen(value);
    do {
        i += tz_ui_stream_push(type, title, value + i, icon);
    } while (i < obuflen);

    return i;
}

size_t
tz_ui_stream_push(tz_ui_cb_type_t type, const char *title, const char *value,
                  tz_ui_icon_t icon)
{
    return tz_ui_stream_pushl(type, title, value, -1, icon);
}

size_t
tz_ui_stream_pushl(tz_ui_cb_type_t type, const char *title, const char *value,
                   ssize_t max, tz_ui_icon_t icon)
{
    tz_ui_stream_t *s = &global.stream;
    size_t          i;

    FUNC_ENTER(("title=%s, value=%s", title, value));
    if (s->full) {
        PRINTF("trying to push in already closed stream display");
        THROW(EXC_UNKNOWN);
    }
#ifdef TEZOS_DEBUG
    int prev_total   = s->total;
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

        PRINTF(
            "[DEBUG] split(value: \"%s\", will_fit: %d, line: %d, "
            "offset: %d)\n",
            &value[offset], will_fit, line, offset);

        strlcpy(s->screens[bucket].body[line], &value[offset], will_fit + 1);

        offset += will_fit;

        line++;
    }

    PRINTF("[DEBUG] tz_ui_stream_pushl(%s, %s, %u)\n", title, value, max);
    PRINTF("[DEBUG]        bucket     %d\n", bucket);
    PRINTF("[DEBUG]        title:     \"%s\"\n", s->screens[bucket].title);
    for (line = 0; line < TZ_UI_STREAM_CONTENTS_LINES; line++)
        PRINTF("[DEBUG]        value[%d]: \"%s\"\n", line,
               s->screens[bucket].body[line]);
    PRINTF("[DEBUG]        total:     %d -> %d\n", prev_total, s->total);
    PRINTF("[DEBUG]        current:   %d -> %d\n", prev_current, s->current);
    PRINTF("[DEBUG]        offset:    %d\n", offset);
    FUNC_LEAVE();

    return offset;
}

tz_ui_cb_type_t
tz_ui_stream_get_type(void)
{
    tz_ui_stream_t *s      = &global.stream;
    size_t          bucket = s->current % TZ_UI_STREAM_HISTORY_SCREENS;

    return s->screens[bucket].type;
}

#ifdef HAVE_BAGL
static void
pred(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));
    if (s->current >= 1
        && s->current >= s->total - TZ_UI_STREAM_HISTORY_SCREENS + 2) {
        s->current--;
    }
    FUNC_LEAVE();
}

static void
succ(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));
    if (s->current < s->total) {
        s->pressed_right = false;
        s->current++;
    }
    FUNC_LEAVE();
}
#endif  // HAVE_BAGL

// View

#ifdef HAVE_BAGL
static unsigned int
cb(unsigned int                         button_mask,
   __attribute__((unused)) unsigned int button_mask_counter)
{
    tz_ui_stream_t *s      = &global.stream;
    size_t          bucket = s->current % TZ_UI_STREAM_HISTORY_SCREENS;
    uint8_t         type   = s->screens[bucket].type;

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
        if (type & TZ_UI_STREAM_CB_MAINMASK) {
            global.step = ST_IDLE;
            ui_home_init();
        }
        break;
    default:
        break;
    }
    FUNC_LEAVE();
    return 0;
}

static const char *
find_icon(tz_ui_icon_t icon)
{
    // clang-format off
    switch (icon) {
    case TZ_UI_ICON_TICK:       return (const char *)&C_icon_validate_14;
    case TZ_UI_ICON_CROSS:      return (const char *)&C_icon_crossmark;
    case TZ_UI_ICON_DASHBOARD:  return (const char *)&C_icon_dashboard_x;
    case TZ_UI_ICON_SETTINGS:   return (const char *)&C_icon_coggle;
    case TZ_UI_ICON_BACK:       return (const char *)&C_icon_back_x;
    default:                    return NULL;
    }
    // clang-format on
}

#define BLACK       0x000000
#define WHITE       0xFFFFFF
#define REGULAR     BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER
#define BOLD        BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER
#define TEXT_HEIGHT 11

static void
redisplay(void)
{
    bagl_element_t init[] = {
      { // background
        .component = {
          .type    = BAGL_RECTANGLE,
          .userid  = 0x00,
          .x       = 0,
          .y       = 0,
          .width   = BAGL_WIDTH, // = 128
          .height  = BAGL_HEIGHT, // = 32 nanos, = 64 nano(sp/x)
          .stroke  = 0,
          .radius  = 0,
          .fill    = BAGL_FILL,
          .fgcolor = BLACK,
          .bgcolor = WHITE,
          .font_id = 0,
          .icon_id = BAGL_GLYPH_NOGLYPH,
        },
        .text = NULL,
      },
      { // left_icon
        .component = {
          .type    = BAGL_ICON,
          .userid  = 0x00,
          .x       = 1, // 1 padding
          .y       = 1, // 1 padding
          .width   = 7, // = C_icon_go_left.width = C_icon_go_forbid.width
          .height  = 7, // = C_icon_go_left.height = C_icon_go_forbid.height
          .stroke  = 0,
          .radius  = 0,
          .fill    = 0,
          .fgcolor = WHITE,
          .bgcolor = BLACK,
          .font_id = 0,
          .icon_id = BAGL_GLYPH_NOGLYPH,
        },
        .text = (const char *)&C_icon_rien,
      },
      { // right_icon
        .component = {
          .type    = BAGL_ICON,
          .userid  = 0x00, // ?
          .x       = BAGL_WIDTH - 7 - 1, // 1 padding
          .y       = 1, // 1 padding
          .width   = 7, // = C_icon_go_right.width
          .height  = 7, // = C_icon_go_right.height
          .stroke  = 0,
          .radius  = 0,
          .fill    = 0,
          .fgcolor = WHITE,
          .bgcolor = BLACK,
          .font_id = 0,
          .icon_id = BAGL_GLYPH_NOGLYPH,
        },
        .text = (const char *)&C_icon_rien,
      },
      { // line0
        .component = {
          .type    = BAGL_LABELINE,
          .userid  = 0x02,
          .x       = 8, // left_icon.x + left_icon.width
          .y       = 8,
          .width   = 112, // right_icon.width - right_icon.y - line0.x
          .height  = TEXT_HEIGHT,
          .stroke  = 0,
          .radius  = 0,
          .fill    = 0,
          .fgcolor = WHITE,
          .bgcolor = BLACK,
          .font_id = BOLD,
          .icon_id = 0,
        },
        .text = global.ux.lines[0],
      },
#ifdef TARGET_NANOS
      { // line1
        .component = {
          .type    = BAGL_LABELINE,
          .userid  = 0x02,
          .x       = 0,
          .y       = 19, // line0.y + 11
          .width   = BAGL_WIDTH,
          .height  = TEXT_HEIGHT,
          .stroke  = 0,
          .radius  = 0,
          .fill    = 0,
          .fgcolor = WHITE,
          .bgcolor = BLACK,
          .font_id = REGULAR,
          .icon_id = 0,
        },
        .text = global.ux.lines[1],
      },
      { // main_icon
        .component = {
          .type    = BAGL_ICON,
          .userid  = 0x00,
          .x       = (BAGL_WIDTH - 16) / 2, // middle
          .y       = BAGL_HEIGHT - 16 - 2, // 2 padding
          .width   = 16,
          .height  = 16,
          .stroke  = 0,
          .radius  = 0,
          .fill    = 0,
          .fgcolor = WHITE,
          .bgcolor = BLACK,
          .font_id = 0,
          .icon_id = BAGL_GLYPH_NOGLYPH,
        },
        .text = (const char *)&C_icon_rien,
      },
#else
      { // line1
        .component = {
          .type    = BAGL_LABELINE,
          .userid  = 0x02,
          .x       = 0,
          .y       = 21, // line0.y + 11 + 2 padding
          .width   = BAGL_WIDTH,
          .height  = TEXT_HEIGHT,
          .stroke  = 0,
          .radius  = 0,
          .fill    = 0,
          .fgcolor = WHITE,
          .bgcolor = BLACK,
          .font_id = REGULAR,
          .icon_id = 0,
        },
        .text = global.ux.lines[1],
      },
      { // line2
        .component = {
          .type    = BAGL_LABELINE,
          .userid  = 0x02,
          .x       = 0,
          .y       = 34, // line1.y + 11 + 2 padding
          .width   = BAGL_WIDTH,
          .height  = TEXT_HEIGHT,
          .stroke  = 0,
          .radius  = 0,
          .fill     = 0,
          .fgcolor = WHITE,
          .bgcolor = BLACK,
          .font_id = REGULAR,
          .icon_id = 0,
        },
        .text = global.ux.lines[2],
      },
      { // line3
        .component = {
          .type    = BAGL_LABELINE,
          .userid  = 0x02,
          .x       = 0,
          .y       = 47, // line2.y + 11 + 2 padding
          .width   = BAGL_WIDTH,
          .height  = TEXT_HEIGHT,
          .stroke  = 0,
          .radius  = 0,
          .fill    = 0,
          .fgcolor = WHITE,
          .bgcolor = BLACK,
          .font_id = REGULAR,
          .icon_id = 0,
        },
        .text = global.ux.lines[3],
      },
      { // line4
        .component = {
          .type    = BAGL_LABELINE,
          .userid  = 0x02,
          .x       = 0,
          .y       = 60, // line3.y + 11 + 2 padding
          .width   = BAGL_WIDTH,
          .height  = TEXT_HEIGHT,
          .stroke  = 0,
          .radius  = 0,
          .fill    = 0,
          .fgcolor = WHITE,
          .bgcolor = BLACK,
          .font_id = REGULAR,
          .icon_id = 0,
        },
        .text = global.ux.lines[4],
      },
      { // main_icon
        .component = {
          .type    = BAGL_ICON,
          .userid  = 0x00,
          .x       = (BAGL_WIDTH - 16) / 2, // middle
          .y       = BAGL_HEIGHT - 16 - 1, // 1 padding
          .width   = 16,
          .height  = 16,
          .stroke  = 0,
          .radius  = 0,
          .fill    = 0,
          .fgcolor = WHITE,
          .bgcolor = BLACK,
          .font_id = 0,
          .icon_id = BAGL_GLYPH_NOGLYPH,
        },
        .text = (const char *)&C_icon_rien,
      },
#endif
    };

    tz_ui_stream_t *s = &global.stream;
    size_t          bucket, i;

    FUNC_ENTER(("void"));

    for (i = 0; i < TZ_SCREEN_LINES_11PX; i++)
        global.ux.lines[i][0] = 0;

    bucket = s->current % TZ_UI_STREAM_HISTORY_SCREENS;

    STRLCPY(global.ux.lines[0], s->screens[bucket].title);
    for (i = 0; i < TZ_UI_STREAM_CONTENTS_LINES; i++) {
        STRLCPY(global.ux.lines[i + 1], s->screens[bucket].body[i]);
    }

    tz_ui_icon_t icon = s->screens[bucket].icon;
    if (icon) {
#ifdef TARGET_NANOS
        global.ux.lines[1][0] = 0;
#endif
        init[sizeof(init) / sizeof(bagl_element_t) - 1].text
            = find_icon(icon);
    }

    /* If we aren't on the first screen, we can go back */
    if (s->current > 0) {
        /* Unless we can't... */
        if (s->current == s->total - TZ_UI_STREAM_HISTORY_SCREENS + 1)
            init[1].text = (const char *)&C_icon_go_forbid;
        else
            init[1].text = (const char *)&C_icon_go_left;
    }
    /* If we aren't full or aren't on the last page, we can go right */
    if (!s->full || s->current < s->total)
        init[2].text = (const char *)&C_icon_go_right;

    DISPLAY(init, cb);
    FUNC_LEAVE();
}

static void
change_screen_left(void)
{
    FUNC_ENTER(("void"));
    pred();
    redisplay();
    FUNC_LEAVE();
}

static void
change_screen_right(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));
    s->pressed_right = true;
    if (s->current == s->total) {
        if (!s->full)
            s->cb(TZ_UI_STREAM_CB_REFILL);
        if (global.step == ST_ERROR) {
            global.step = ST_IDLE;
            ui_home_init();
            return;
        }
    }
    // go back to the data screen
    succ();
    redisplay();
    FUNC_LEAVE();
}
#endif  // HAVE_BAGL

void
tz_ui_stream_start(void)
{
    FUNC_ENTER(("void"));
#ifdef HAVE_BAGL
    redisplay();
#endif
    FUNC_LEAVE();
}

#ifdef HAVE_BAGL
void
tz_ui_stream(void)
{
    FUNC_ENTER(("void"));

    tz_ui_stream_t *s = &global.stream;
    if (s->pressed_right)
        succ();

    redisplay();
    FUNC_LEAVE();
}
#endif
