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

#define BUFF_START ((char *)(s->buffer))
#define BUFF_END   ((char *)(s->buffer) + BUFF_LEN)

void ui_strings_init(void) {
  tz_ui_strings_t *s = &global.stream.strings;

  memset(s->buffer, BUFF_LEN, '\0');
  s->start = BUFF_START;
  s->end = BUFF_END;
  s->internal_end = BUFF_START;
  s->count = 0;
}

/* Prototypes */
void ui_strings_push(const char *str, size_t len, char *out);
void ui_strings_drop(char *str);
void ui_strings_fit_up_to(size_t len, char *write_start, char *write_end);
void ui_strings_can_fit(size_t len, bool *can_fit);
bool ui_strings_is_empty();

/* Definitions */

/* @param len: we want to a string s of strlen(s) == len
   @param write_start: this is where we'll start writing this string in the
   buffer
   @param write_end: this is the pointer to the null char of the string
*/
void
ui_strings_fit_up_to(size_t len, char *write_start, char *write_end)
{
    tz_ui_strings_t *s = &global.stream.strings;
    TZ_PREAMBLE(("len=%d, start=%p, end=%p", len, s->start, s->end));

    /* Preconditions */
    TZ_ASSERT(EXC_MEMORY_ERROR, (write_start == NULL && write_end == NULL));
    TZ_ASSERT(EXC_MEMORY_ERROR, (len > 0));
    /* Internal checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (s->end <= s->internal_end));

    if (ui_strings_is_empty()) {
        TZ_ASSERT(EXC_MEMORY_ERROR, (len < BUFF_LEN));

        write_start = s->start;
        write_end   = s->start + len;

        TZ_SUCCEED();
    }

    /* Buffer not empty */

    if (s->start >= s->end) {
        write_start = s->end;
        write_end   = s->start - 1;

        TZ_SUCCEED();
    }

    /* start < end */
    size_t bytes_at_start = (s->start - BUFF_START);
    size_t bytes_at_end   = (BUFF_END - s->end);

    if (bytes_at_end > len || bytes_at_end >= bytes_at_start) {
        write_start = s->end;
        write_end   = s->end + len;
    } else {
        write_start = BUFF_START;
        write_end   = BUFF_START + bytes_at_start - 1;
    }

    TZ_POSTAMBLE;
}

void
ui_strings_can_fit(size_t len, bool *can_fit)
{
    char *ws = NULL;
    char *we = NULL;

    TZ_PREAMBLE(("len=%d", len));
    TZ_ASSERT(EXC_MEMORY_ERROR, (can_fit == NULL));

    TZ_CHECK(ui_strings_fit_up_to(len, ws, we));

    *can_fit = (we - ws >= (int)len);

    TZ_POSTAMBLE;
}

/* @param in: ptr to char[] to copy into the buffer
   @param in_len: number of of chars to copy. in_len <= strlen(in)
   @param out: will be set to the start of the char[] in the buffer

   // TODO: for future, when appending is a possibility
   @param out_len: strlen(out) 0 < out_len <= in_len
*/
void
ui_strings_push(const char *in, size_t len, char *out)
{
    tz_ui_strings_t *s = &global.stream.strings;
    TZ_PREAMBLE(("in=%p, len=%d, start=%p, end=%p", in, len, s->start, s->end));

    /* Preconditions */
    TZ_ASSERT(EXC_MEMORY_ERROR, (out == NULL));
    TZ_ASSERT_NOTNULL(in);
    TZ_ASSERT(EXC_MEMORY_ERROR, (len > 0));
    /* Internal checks */
    /* TZ_ASSERT(EXC_MEMORY_ERROR, (start < internal_end && count > 0)); */
    TZ_ASSERT(EXC_MEMORY_ERROR, (s->end <= s->internal_end));

    char *ws = NULL;
    char *we = NULL;
    TZ_CHECK(ui_strings_fit_up_to(len, ws, we));

    TZ_ASSERT(EXC_MEMORY_ERROR, (we - ws >= (int)len));

    strlcpy(ws, in, we - ws + 1);
    s->count++;

    s->end = we + 1;

    if (s->end > s->internal_end)
        s->internal_end = s->end;

    TZ_POSTAMBLE;
}

void
ui_strings_drop(char *in)
{
    tz_ui_strings_t *s = &global.stream.strings;
    TZ_PREAMBLE(("in=%p, start=%p, end=%p", in, s->start, s->end));

    /* argument checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (in == s->start));
    TZ_ASSERT(EXC_MEMORY_ERROR, (!ui_strings_is_empty()));
    /* Internal checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (s->start < s->internal_end));
    TZ_ASSERT(EXC_MEMORY_ERROR, (s->end <= s->internal_end));

    size_t len = strlen(in);
    TZ_ASSERT(EXC_MEMORY_ERROR, (len > 0));

    char *new = in + len + 1;
    TZ_ASSERT(EXC_MEMORY_ERROR, (new <= s->internal_end));

    in = NULL;
    s->count--;
    memset(s->start, len, '\0');

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
}

bool
ui_strings_is_empty()
{
    tz_ui_strings_t *s = &global.stream.strings;
    return s->start == s->end && s->count == 0; /* check COUNT is zero! */
}
