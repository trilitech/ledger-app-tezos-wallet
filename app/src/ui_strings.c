/* Tezos Ledger application - Generic stream display

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

#include <io.h>
#include <stdbool.h>

#include "globals.h"
#include "ui_strings.h"

/* Allow future unit tests to override */
#ifndef UI_STRINGS
#define UI_STRINGS &global.stream.strings
#endif

#define BUFF_START ((char *)(s->buffer))
#define BUFF_END   ((char *)(s->buffer) + BUFF_LEN)

/* Prototypes */
void   ui_strings_init(void);
void   ui_strings_push(const char *str, size_t len, char **out);
void   ui_strings_drop(char **str);
void   ui_strings_drop_last(char **str);
size_t ui_strings_fit_up_to(size_t len, char **write_start);
void   ui_strings_can_fit(size_t len, bool *can_fit);
bool   ui_strings_is_empty();

#ifdef TEZOS_DEBUG
void ui_strings_print();
#define PRINT_STRINGS ui_strings_print()
#else
#define PRINT_STRINGS
#endif

/* Definitions */
void
ui_strings_init(void)
{
    tz_ui_strings_t *s = UI_STRINGS;

    memset(s->buffer, '\0', BUFF_LEN);
    s->start        = BUFF_START;
    s->end          = BUFF_START;
    s->internal_end = BUFF_START;
    s->count        = 0;
}

/* @param len: we want to a string s of strlen(s) == len
   @param write_start: this is where we'll start writing this string in the
   buffer
   @return out_len: this is the length of the string we can fit. out_len <=
   len
*/
size_t
ui_strings_fit_up_to(size_t len, char **write_start)
{
    tz_ui_strings_t *s       = UI_STRINGS;
    size_t           out_len = 0;
    TZ_PREAMBLE(
        ("len=%d, start=%p, end=%p", len, s->start, s->end, s->internal_end));

    /* Preconditions */
    TZ_ASSERT(EXC_MEMORY_ERROR, (*write_start == NULL));
    TZ_ASSERT(EXC_MEMORY_ERROR, (len > 0));
    /* Internal checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (s->end <= s->internal_end));

    if (ui_strings_is_empty()) {
        TZ_ASSERT(EXC_MEMORY_ERROR, (len < BUFF_LEN));

        *write_start = s->start;
        out_len      = len;

        TZ_SUCCEED();
    }

    /* Buffer not empty */

    if (s->start > s->end) {
        *write_start = s->end;
        out_len      = MIN((size_t)(s->start - 1 - s->end), len);
    } else if (s->start < s->end) {
        /* start < end */
        size_t chars_at_start = MIN((size_t)(s->start - BUFF_START), len + 1);
        chars_at_start
            = chars_at_start > 0 ? chars_at_start - 1 : chars_at_start;

        size_t chars_at_end = MIN((size_t)(BUFF_END - s->end), len + 1);
        chars_at_end = chars_at_end > 0 ? chars_at_end - 1 : chars_at_end;

        if (chars_at_end == len || chars_at_end >= chars_at_start) {
            *write_start = s->end;
            out_len      = chars_at_end;
        } else {
            *write_start = BUFF_START;
            out_len      = chars_at_start;
        }
    }

    TZ_POSTAMBLE;
    PRINTF("[DEBUG] ws=%p out_len=%d\n", *write_start, out_len);
    return out_len;
}

void
ui_strings_can_fit(size_t len, bool *can_fit)
{
    char  *ws = NULL;
    size_t out_len;

    TZ_PREAMBLE(("len=%d", len));

    TZ_CHECK(out_len = ui_strings_fit_up_to(len, &ws));

    *can_fit = (out_len == len);

    TZ_POSTAMBLE;
}

/* @param in: ptr to char[] to copy into the buffer
   @param in_len: number of of chars to copy. in_len <= strlen(in)
   @param out: will be set to the start of the char[] in the buffer

   // TODO: for future, when appending is a possibility
   @param out_len: strlen(out) 0 < out_len <= in_len
*/
void
ui_strings_push(const char *in, size_t len, char **out)
{
    tz_ui_strings_t *s = UI_STRINGS;
    TZ_PREAMBLE(("'%s' | in=%p, len=%d, start=%p, end=%p, internal_end=%p",
                 in, in, len, s->start, s->end, s->internal_end));
    PRINT_STRINGS;

    /* Preconditions */
    TZ_ASSERT(EXC_MEMORY_ERROR, (*out == NULL));
    TZ_ASSERT_NOTNULL(in);
    TZ_ASSERT(EXC_MEMORY_ERROR, (len > 0));
    /* Internal checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (s->end <= s->internal_end));

    char  *ws = NULL;
    size_t out_len;
    TZ_CHECK(out_len = ui_strings_fit_up_to(len, &ws));
    PRINTF("[DEBUG] Found space from %p to %p (for %d chars)\n", ws,
           ws + out_len, out_len);

    TZ_ASSERT(EXC_MEMORY_ERROR, (out_len == len));

    strlcpy(ws, in, len + 1);
    s->count++;

    s->end = ws + len + 1;

    if (s->end > s->internal_end)
        s->internal_end = s->end;

    *out = ws;
    PRINTF("[DEBUG] Pushed '%s' to %p\n", *out, *out);

    TZ_POSTAMBLE;
    PRINT_STRINGS;
}

void
ui_strings_drop(char **in)
{
    tz_ui_strings_t *s = UI_STRINGS;
    TZ_PREAMBLE(("in=%p, start=%p, end=%p", *in, s->start, s->end));
    PRINT_STRINGS;

    /* argument checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (*in == s->start));
    TZ_ASSERT(EXC_MEMORY_ERROR, (!ui_strings_is_empty()));
    /* Internal checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (s->start < s->internal_end));
    TZ_ASSERT(EXC_MEMORY_ERROR, (s->end <= s->internal_end));

    size_t len = strlen(*in);
    TZ_ASSERT(EXC_MEMORY_ERROR, (len > 0));

    char *new = *in + len + 1;
    TZ_ASSERT(EXC_MEMORY_ERROR, (new <= s->internal_end));

    PRINTF("[DEBUG] zeroing %p (%d) (%s)\n", s->start, len, s->start);
    memset(s->start, '\0', len);
    *in = NULL;
    s->count--;

    if (new < s->internal_end) {
        TZ_ASSERT(EXC_MEMORY_ERROR, (*new != '\0'));
        s->start = new;
        TZ_SUCCEED();
    }

    /* This was the last string in the region */

    s->start = BUFF_START;

    if (s->end == s->internal_end) {
        /* This was the last string in the ring buffer */
        s->end = BUFF_START;
        TZ_ASSERT(EXC_MEMORY_ERROR, (ui_strings_is_empty()));
    }

    s->internal_end = s->end;

    TZ_POSTAMBLE;
    PRINT_STRINGS;
}

void
ui_strings_drop_last(char **in)
{
    tz_ui_strings_t *s = UI_STRINGS;
    TZ_PREAMBLE(("in=%p, start=%p, end=%p", *in, s->start, s->end));
    PRINT_STRINGS;

    /* argument checks */
    TZ_ASSERT_NOTNULL(*in);

    size_t len = strlen(*in);
    TZ_ASSERT(EXC_MEMORY_ERROR, (len > 0));
    TZ_ASSERT(EXC_MEMORY_ERROR, (!ui_strings_is_empty()));
    TZ_ASSERT(EXC_MEMORY_ERROR, (*in + len == s->end - 1));
    /* Internal checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (s->start < s->internal_end));
    TZ_ASSERT(EXC_MEMORY_ERROR, (s->end <= s->internal_end));

    PRINTF("[DEBUG] zeroing %p (%d) (%s)\n", *in, len, *in);
    memset(*in, '\0', len);

    if (*in == BUFF_START || *(*in - 1) == '\0') {
        s->count--;
        s->end = *in;
    } else {
        s->end = *in + 1;
    }

    *in = NULL;

    if (s->end == BUFF_START && !ui_strings_is_empty()) {
        s->end = s->internal_end;
    } else if (s->start <= s->end) {
        s->internal_end = s->end;
    }

    PRINTF("[DEBUG] s=%p e=%p ie=%p", s->start, s->end, s->internal_end);
    TZ_POSTAMBLE;
    PRINT_STRINGS;
}

bool
ui_strings_is_empty()
{
    tz_ui_strings_t *s = UI_STRINGS;
    return s->start == s->end && s->count == 0; /* check COUNT is zero! */
}

#ifdef TEZOS_DEBUG
void
ui_strings_next(char **p)
{
    tz_ui_strings_t *s = UI_STRINGS;

    if (*p >= s->start) {
        size_t len  = strlen(*p);
        char  *next = *p + len + 1;

        if (next < s->internal_end) {
            *p = next;
        } else if (s->start >= s->end) {
            *p = BUFF_START;
        } else {
            *p = NULL;
        }
    } else if (*p < s->end) {
        size_t len  = strlen(*p);
        char  *next = *p + len + 1;

        if (next < s->end) {
            *p = next;
        } else {
            *p = NULL;
        }
    } else {
        *p = NULL;
    }
}

void
ui_strings_print()
{
    tz_ui_strings_t *s = UI_STRINGS;

    if (ui_strings_is_empty()) {
        PRINTF("[DEBUG] START\t\t%p\n[DEBUG] END\t\t%p\n", BUFF_START,
               BUFF_END);
        return;
    }

    if (s->start == BUFF_START) {
        char *p = s->start;
        PRINTF("[DEBUG] START\tstart->\t%p %s\n", s->start, s->start);
        while (true) {
            ui_strings_next(&p);
            if (!p)
                break;
            PRINTF("[DEBUG] \t\t%p %s\n", p, p);
        }
        if (s->end < BUFF_END) {
            PRINTF("[DEBUG] \tend  ->\t%p\n[DEBUG] END\t\t%p\n", s->end,
                   BUFF_END);
        } else {
            PRINTF("[DEBUG] END\tend ->\t%p\n", s->end);
        }
    } else if (s->start >= s->end) {
        char *p = BUFF_START;
        PRINTF("[DEBUG] START\t\t%p %s\n", p, p);
        while (true) {
            ui_strings_next(&p);

            if (!p) {
                PRINTF("[DEBUG] \tend  ->\t%p\n", s->end);
                break;
            }
            PRINTF("[DEBUG] \t\t%p %s\n", p, p);
        }

        p = s->start;
        PRINTF("[DEBUG] \tstart->\t%p %s\n", p, p);
        while (true) {
            ui_strings_next(&p);
            if (p >= s->start)
                PRINTF("[DEBUG] \t\t%p %s\n", p, p);
            else
                break;
        }
        PRINTF("[DEBUG] END\t\t%p\n", BUFF_END);
    } else {
        char *p = s->start;
        PRINTF("[DEBUG] START\t\t%p\n", BUFF_START);
        PRINTF("[DEBUG] \tstart->\t%p %s\n", p, p);
        while (true) {
            ui_strings_next(&p);
            if (!p)
                break;
            PRINTF("[DEBUG] \t\t%p %s\n", p, p);
        }
        if (s->end < BUFF_END) {
            PRINTF("[DEBUG] \tend  ->\t%p\n[DEBUG] END\t\t%p\n", s->end,
                   BUFF_END);
        } else {
            PRINTF("[DEBUG] END\tend ->\t%p\n", s->end);
        }
    }
}
#endif
