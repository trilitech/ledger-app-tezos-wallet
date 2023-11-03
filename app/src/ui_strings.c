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

#define BUFF_LEN 512
static char buff[BUFF_LEN] = {0};
#define BUFF_START ((char *)&buff)
#define BUFF_END   ((char *)(&buff + BUFF_LEN))
static char  *start        = BUFF_START;
static char  *end          = BUFF_START;
static char  *internal_end = BUFF_START;
static size_t count        = 0;

/* Prototypes */
void ui_strings_push(const char *str, size_t len, char *out);
void ui_strings_drop(char *str);
void ui_strings_fit_up_to(size_t len, char *write_start, char *write_end);
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
    TZ_PREAMBLE(("len=%d, start=%p, end=%p", len, start, end));

    /* Preconditions */
    TZ_ASSERT(EXC_MEMORY_ERROR, (write_start == NULL && write_end == NULL));
    TZ_ASSERT(EXC_MEMORY_ERROR, (len > 0));
    /* Internal checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (end <= internal_end));

    if (ui_strings_is_empty()) {
        TZ_ASSERT(EXC_MEMORY_ERROR, (len < BUFF_LEN));

        write_start = start;
        write_end   = start + len;

        TZ_SUCCEED();
    }

    /* Buffer not empty */

    if (start >= end) {
        write_start = end;
        write_end   = start - 1;

        TZ_SUCCEED();
    }

    /* start < end */
    size_t bytes_at_start = (start - BUFF_START);
    size_t bytes_at_end   = (BUFF_END - end);

    if (bytes_at_end > len || bytes_at_end >= bytes_at_start) {
        write_start = end;
        write_end   = end + len;
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
    TZ_PREAMBLE(("in=%p, len=%d, start=%p, end=%p", in, len, start, end));

    /* Preconditions */
    TZ_ASSERT(EXC_MEMORY_ERROR, (out == NULL));
    TZ_ASSERT_NOTNULL(in);
    TZ_ASSERT(EXC_MEMORY_ERROR, (len > 0));
    /* Internal checks */
    /* TZ_ASSERT(EXC_MEMORY_ERROR, (start < internal_end && count > 0)); */
    TZ_ASSERT(EXC_MEMORY_ERROR, (end <= internal_end));

    char *ws = NULL;
    char *we = NULL;
    TZ_CHECK(ui_strings_fit_up_to(len, ws, we));

    TZ_ASSERT(EXC_MEMORY_ERROR, (we - ws >= (int)len));

    strlcpy(ws, in, we - ws + 1);

    end = we + 1;

    if (end > internal_end)
        internal_end = end;

    TZ_POSTAMBLE;
}

void
ui_strings_drop(char *in)
{
    TZ_PREAMBLE(("in=%p, start=%p, end=%p", in, start, end));

    /* argument checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (in == start));
    TZ_ASSERT(EXC_MEMORY_ERROR, (!ui_strings_is_empty()));
    /* Internal checks */
    TZ_ASSERT(EXC_MEMORY_ERROR, (start < internal_end));
    TZ_ASSERT(EXC_MEMORY_ERROR, (end <= internal_end));

    size_t len = strlen(in);
    TZ_ASSERT(EXC_MEMORY_ERROR, (len > 0));

    char *new = in + len + 1;
    TZ_ASSERT(EXC_MEMORY_ERROR, (new <= internal_end));

    in = NULL;
    count--;
    memset(start, len, '\0');

    if (new < internal_end) {
        TZ_ASSERT(EXC_MEMORY_ERROR, (*new != '\0'));
        start = new;
        TZ_SUCCEED();
    }

    /* This was the last string in the region */

    start = BUFF_START;

    if (end == internal_end) {
        /* This was the last string in the ring buffer */
        end = BUFF_START;
        TZ_ASSERT(EXC_MEMORY_ERROR, (ui_strings_is_empty()));
    }

    internal_end = end;

    TZ_POSTAMBLE;
}

bool
ui_strings_is_empty()
{
    return start == end && count == 0; /* check COUNT is zero! */
}
