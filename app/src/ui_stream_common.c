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
#include "exception.h"
#include "globals.h"
#include "ui_strings.h"
#include "ui_stream.h"

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

void
push_str(const char *text, size_t len, char **out)
{
    bool can_fit = false;

    TZ_PREAMBLE(("%s", text));

    TZ_CHECK(ui_strings_can_fit(len, &can_fit));
    while (!can_fit) {
        TZ_CHECK(drop_last_screen());
        TZ_CHECK(ui_strings_can_fit(len, &can_fit));
    }

    TZ_CHECK(ui_strings_push(text, len, out));

    TZ_POSTAMBLE;
}

void
tz_ui_stream_close(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("full=%d", s->full));
    if (s->full) {
        PRINTF("trying to close already closed stream display");
        THROW(EXC_UNKNOWN);
    }
    s->full = true;

    FUNC_LEAVE();
}
