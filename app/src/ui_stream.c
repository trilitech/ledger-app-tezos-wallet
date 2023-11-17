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
#include "exception.h"
#include "ui_strings.h"

//! Init array consists of TZ_SCREEN_LINES + background + left & right arrow +
//! picture.
#define UI_INIT_ARRAY_LEN (4 + TZ_SCREEN_LINES_11PX)

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

void drop_last_screen(void);
void push_str(const char *, size_t, char **);

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
    s->last    = 0;

    ui_strings_init();

    FUNC_LEAVE();
}
#endif

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

size_t
tz_ui_stream_push_all(tz_ui_cb_type_t cb_type, const char *title,
                      const char *value, tz_ui_layout_type_t layout_type,
                      tz_ui_icon_t icon)
{
    size_t obuflen;
    size_t i = 0;

    FUNC_ENTER(("cb_type=%d title=%s value=%s", cb_type, title, value));

    obuflen = strlen(value);
    do {
        i += tz_ui_stream_push(cb_type, title, value + i, layout_type, icon);
        PRINTF("[DEBUG] pushed %d in total\n", i);
    } while (i < obuflen);

    FUNC_LEAVE();
    return i;
}

size_t
tz_ui_stream_push(tz_ui_cb_type_t cb_type, const char *title,
                  const char *value, tz_ui_layout_type_t layout_type,
                  tz_ui_icon_t icon)
{
    return tz_ui_stream_pushl(cb_type, title, value, -1, layout_type, icon);
}

tz_ui_cb_type_t
tz_ui_stream_get_cb_type(void)
{
    tz_ui_stream_t *s      = &global.stream;
    size_t          bucket = s->current % TZ_UI_STREAM_HISTORY_SCREENS;

    return s->screens[bucket].cb_type;
}

#ifdef HAVE_BAGL
static void
pred(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));
    if (s->current >= 1 && s->current > s->last) {
        s->current--;
    }
    FUNC_LEAVE();
}

static void
succ(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("current=%d total=%d", s->current, s->total));
    if (s->current < s->total) {
        s->pressed_right = false;
        s->current++;
    }
    FUNC_LEAVE();
}

// View

static unsigned int
cb(unsigned int                         button_mask,
   __attribute__((unused)) unsigned int button_mask_counter)
{
    tz_ui_stream_t *s       = &global.stream;
    size_t          bucket  = s->current % TZ_UI_STREAM_HISTORY_SCREENS;
    uint8_t         cb_type = s->screens[bucket].cb_type;

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
        if (cb_type)
            s->cb(cb_type);
        if (cb_type & TZ_UI_STREAM_CB_MAINMASK) {
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
    case TZ_UI_ICON_EYE:        return (const char *)&C_icon_eye;
    default:                    return NULL;
    }
    // clang-format on
}

static void
display_init(bagl_element_t init[UI_INIT_ARRAY_LEN])
{
    tz_ui_stream_t *s = &global.stream;
    FUNC_ENTER(("void"));

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

    DISPLAY(init, cb, UI_INIT_ARRAY_LEN)
    FUNC_LEAVE();
}

/**
 * Display the screen with given layout.
 * layout -
 * icon_pos - Position of icon in init array.
 */
static void
redisplay_screen(tz_ui_layout_type_t layout, uint8_t icon_pos)
{
    TZ_PREAMBLE(("void"));
    tz_ui_stream_t *s = &global.stream;
    size_t          bucket;
    bucket            = s->current % TZ_UI_STREAM_HISTORY_SCREENS;
    tz_ui_icon_t icon = s->screens[bucket].icon;

    bagl_element_t init[] = {
  //  {type, userid, x, y, width, height, stroke, radius,
  //   fill, fgcolor, bgcolor, font_id, icon_id}, text/icon
        {{BAGL_RECTANGLE, 0x00, 0, 0, 128, BAGL_HEIGHT, 0, 0, BAGL_FILL,
          0x000000, 0xFFFFFF, 0, 0},
         NULL                      },
        {{BAGL_ICON, 0x00, 1, 1, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
          BAGL_GLYPH_NOGLYPH},
         (const char *)&C_icon_rien},
        {{BAGL_ICON, 0x00, 120, 1, 7, 7, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
          BAGL_GLYPH_NOGLYPH},
         (const char *)&C_icon_rien},
        {{BAGL_LABELINE, 0x02, 8, 8, 112, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
          BOLD, 0},
         s->screens[bucket].title  },
#ifdef TARGET_NANOS
        {{BAGL_LABELINE, 0x02, 0, 19, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
          REGULAR, 0},
         s->screens[bucket].body[0]},
        {{BAGL_ICON, 0x00, 56, 14, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
          BAGL_GLYPH_NOGLYPH},
         (const char *)&C_icon_rien},
#else
        {{BAGL_LABELINE, 0x02, 0, 21, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
          REGULAR, 0},
         s->screens[bucket].body[0]},
        {{BAGL_LABELINE, 0x02, 0, 34, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
          REGULAR, 0},
         s->screens[bucket].body[1]},
        {{BAGL_LABELINE, 0x02, 0, 47, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
          REGULAR, 0},
         s->screens[bucket].body[2]},
        {{BAGL_LABELINE, 0x02, 0, 60, 128, 11, 0, 0, 0, 0xFFFFFF, 0x000000,
          REGULAR, 0},
         s->screens[bucket].body[3]},
        {{BAGL_ICON, 0x00, 56, 47, 16, 16, 0, 0, 0, 0xFFFFFF, 0x000000, 0,
          BAGL_GLYPH_NOGLYPH},
         (const char *)&C_icon_rien},
#endif
    };

    const uint8_t txt_start_line
        = 3;  /// first three lines are for black rectangle, left screen icon
              /// and right screen icon.

    if (layout == TZ_UI_LAYOUT_BP || layout == TZ_UI_LAYOUT_HOME_BP) {
        // Change the contents to bold.
        for (int i = txt_start_line + 1; i < icon_pos; i++) {
            init[i].component.font_id = BOLD;
        }
    } else if (layout == TZ_UI_LAYOUT_NP || layout == TZ_UI_LAYOUT_HOME_NP) {
        // Set title to Regular.
        init[txt_start_line].component.font_id = REGULAR;
    } else if (layout == TZ_UI_LAYOUT_HOME_PB) {
        // Icon will be at txt_start_line.
        // modify the x,y coordinates for index txt_start_line to end.
        init[txt_start_line].component   = init[icon_pos].component;
        init[txt_start_line].component.x = BAGL_WIDTH / 2 - 8;
#ifdef TARGET_NANOS
        init[txt_start_line].component.y = BAGL_HEIGHT / 2 - 14;
#else
        init[txt_start_line].component.y = BAGL_HEIGHT / 2 - 20;
#endif
        icon_pos = txt_start_line;
        for (int i = txt_start_line + 1; i < UI_INIT_ARRAY_LEN; i++) {
            init[i].component         = init[icon_pos + 1].component;
            init[i].component.font_id = BOLD;
            if (i == txt_start_line + 1)
                init[i].text = s->screens[bucket].title;
            else
                init[i].text = s->screens[bucket].body[i - 5];
            init[i].component.x = 8;
            init[i].component.y
                = init[txt_start_line].component.y + 16 + 8 + ((i - 4) * 12);
            init[i].component.width = 112;
        }
    }

    if (icon) {
        init[icon_pos].text = find_icon(icon);
#ifdef TARGET_NANOS
        // Make sure text does not overflow on icon line in non-PB layouts.
        if (layout != TZ_UI_LAYOUT_HOME_PB)
            init[icon_pos - 1].text = NULL;
#endif
    }

    // if the screen layout type is home , set the left and right arrows to
    // middle of screen.
    if (layout & TZ_UI_LAYOUT_HOME_MASK) {
        init[1].component.y = BAGL_HEIGHT / 2 - 3;
        init[2].component.y = BAGL_HEIGHT / 2 - 3;
        // as icon_pos = txt_start_line in TZ_UI_LAYOUT_HOME_PB layout,
        // following changes dont affect it.
        for (int i = txt_start_line; i < icon_pos; i++) {
            init[i].component.x     = 8;
            init[i].component.width = 112;
            init[i].component.y
                = BAGL_HEIGHT / 2 - 3 + ((i - txt_start_line) * 13);
        }
    }

    display_init(init);
    TZ_POSTAMBLE;
}

static void
redisplay(void)
{
    TZ_PREAMBLE(("void"));

    tz_ui_stream_t *s        = &global.stream;
    size_t          bucket   = s->current % TZ_UI_STREAM_HISTORY_SCREENS;
    uint8_t         icon_pos = UI_INIT_ARRAY_LEN - 1;
    // clang-format off
    redisplay_screen(s->screens[bucket].layout_type, icon_pos);
    // clang-format on
    TZ_POSTAMBLE;
}

static void
change_screen_left(void)
{
    FUNC_ENTER(("void"));
    pred();
    redisplay();
    FUNC_LEAVE();
}

void
change_screen_right(void)
{
    tz_ui_stream_t *s = &global.stream;

    TZ_PREAMBLE(("void"));
    s->pressed_right = true;
    if (!s->full && s->current == s->total) {
        PRINTF("[DEBUG] Looping in change_screen_right\n");
        s->cb(TZ_UI_STREAM_CB_REFILL);
        PRINTF("[DEBUG] step=%d\n", global.keys.apdu.sign.step);

        if (global.step == ST_ERROR) {
            global.step = ST_IDLE;
            ui_home_init();
            return;
        }
    }
    // go back to the data screen
    succ();

    redisplay();
    TZ_POSTAMBLE;
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

/* pushl mechanism */
size_t
tz_ui_stream_pushl(tz_ui_cb_type_t cb_type, const char *title,
                   const char *value, ssize_t max,
                   tz_ui_layout_type_t layout_type, tz_ui_icon_t icon)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("title=%s, value=%s", title, value));
    if (s->full) {
        PRINTF("trying to push in already closed stream display");
        THROW(EXC_UNKNOWN);
    }
#ifdef TEZOS_DEBUG
    int prev_total   = s->total;
    int prev_current = s->current;
    int prev_last    = s->last;
#endif

    s->total++;
    int bucket = s->total % TZ_UI_STREAM_HISTORY_SCREENS;

    if (s->total > 0
        && (s->current % TZ_UI_STREAM_HISTORY_SCREENS) == bucket) {
        PRINTF(
            "[ERROR] PANIC!!!! Overwriting current screen, some bad things "
            "are happening\n");
    }

    /* drop the previous screen text in our bucket */
    if (s->total > 0 && bucket == (s->last % TZ_UI_STREAM_HISTORY_SCREENS))
        drop_last_screen();

    push_str(title, strlen(title), &s->screens[bucket].title);

    // Ensure things fit on one line
    size_t length = strlen(value);
    size_t offset = 0;

    if (max != -1)
        length = MIN(length, (size_t)max);

    s->screens[bucket].cb_type     = cb_type;
    s->screens[bucket].layout_type = layout_type;
    s->screens[bucket].icon        = icon;

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

        push_str(&value[offset], will_fit, &s->screens[bucket].body[line]);

        offset += will_fit;

        line++;
    }

    PRINTF("[DEBUG] tz_ui_stream_pushl(%s, %s, %u)\n", title, value, max);
    PRINTF("[DEBUG]        bucket     %d\n", bucket);
    PRINTF("[DEBUG]        title:     \"%s\"\n", s->screens[bucket].title);
    for (line = 0; line < TZ_UI_STREAM_CONTENTS_LINES; line++)
        if (s->screens[bucket].body[line]) {
            PRINTF("[DEBUG]        value[%d]:  \"%s\"\n", line,
                   s->screens[bucket].body[line]);
        } else {
            PRINTF("[DEBUG]        value[%d]:  \"\"\n", line);
        }
    PRINTF("[DEBUG]        total:     %d -> %d\n", prev_total, s->total);
    PRINTF("[DEBUG]        current:   %d -> %d\n", prev_current, s->current);
    PRINTF("[DEBUG]        last:      %d -> %d\n", prev_last, s->last);
    PRINTF("[DEBUG]        offset:    %d\n", offset);
    FUNC_LEAVE();

    return offset;
}

void
drop_last_screen(void)
{
    tz_ui_stream_t *s      = &global.stream;
    size_t          bucket = s->last % TZ_UI_STREAM_HISTORY_SCREENS;

    TZ_PREAMBLE(("last: %d", s->last));

    size_t i;
    if (s->screens[bucket].title)
        TZ_CHECK(ui_strings_drop(&s->screens[bucket].title));
    for (i = 0; i < TZ_UI_STREAM_CONTENTS_LINES; i++) {
        if (s->screens[bucket].body[i]) {
            TZ_CHECK(ui_strings_drop(&s->screens[bucket].body[i]));
        }
    }
    s->last++;

    TZ_POSTAMBLE;
}
#endif

void
push_str(const char *text, size_t len, char **out)
{
    bool can_fit = false;

    TZ_PREAMBLE(("%s", text));

    if (len == 0) {
        *out = NULL;
        TZ_SUCCEED();
    }

    TZ_CHECK(ui_strings_can_fit(len, &can_fit));
    while (!can_fit) {
        TZ_CHECK(drop_last_screen());
        TZ_CHECK(ui_strings_can_fit(len, &can_fit));
    }

    TZ_CHECK(ui_strings_push(text, len, out));

    TZ_POSTAMBLE;
}
