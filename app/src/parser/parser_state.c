/* Tezos Embedded C parser for Ledger - Full parser state definition and
   helpers

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

#include "parser_state.h"

#define TZ_LABEL(_x) \
    case TZ_##_x:    \
        return #_x
#define BLO_LABEL(_x) \
    case TZ_BLO_##_x: \
        return #_x

const char *
tz_parser_result_name(tz_parser_result code)
{
    // clang-format off
    switch (code) {
    TZ_LABEL(CONTINUE);
    TZ_LABEL(BREAK);
    BLO_LABEL(DONE);
    BLO_LABEL(FEED_ME);
    BLO_LABEL(IM_FULL);
    TZ_LABEL(ERR_INVALID_TAG);
    TZ_LABEL(ERR_INVALID_OP);
    TZ_LABEL(ERR_INVALID_DATA);
    TZ_LABEL(ERR_UNSUPPORTED);
    TZ_LABEL(ERR_TOO_LARGE);
    TZ_LABEL(ERR_TOO_DEEP);
    TZ_LABEL(ERR_INVALID_STATE);
    default:
        return "Unknown";
    }
    // clang-format on
}

void
tz_parser_init(tz_parser_state *state)
{
    state->errno                       = TZ_CONTINUE;
    state->ofs                         = 0;
    state->field_info.field_name[0]    = 0;
    state->field_info.is_field_complex = false;
    state->field_info.field_index      = 0;
}

void
tz_parser_flush(tz_parser_state *st, char *obuf, size_t olen)
{
    tz_parser_flush_up_to(st, obuf, olen, olen);
}

void
tz_parser_flush_up_to(tz_parser_state *st, char *obuf, size_t olen,
                      size_t up_to)
{
    tz_parser_regs *regs = &st->regs;

    regs->obuf = obuf;
    regs->oofs = 0;
    regs->olen = olen;

    size_t len = strlen(regs->obuf + up_to);
    regs->oofs += len;
    regs->olen -= len;

    memmove(regs->obuf, regs->obuf + up_to, len);
    memset(regs->obuf + regs->oofs, 0x0, regs->olen);
}

void
tz_parser_refill(tz_parser_state *st, uint8_t *ibuf, size_t ilen)
{
    tz_parser_regs *regs = &st->regs;

    regs->ibuf = ibuf;
    regs->iofs = 0;
    regs->ilen = ilen;
}

tz_parser_result
tz_parser_set_errno(tz_parser_state *state, tz_parser_result code)
{
    state->errno = ((code == TZ_BREAK) ? TZ_CONTINUE : code);
    return code;
}

tz_parser_result
tz_parser_put(tz_parser_state *state, char c)
{
    tz_parser_regs *regs = &state->regs;

    if (regs->olen < 1)
        tz_stop(IM_FULL);
    regs->obuf[regs->oofs] = c;
    regs->oofs++;
    regs->olen--;
    tz_continue;
}

tz_parser_result
tz_parser_read(tz_parser_state *state, uint8_t *r)
{
    tz_parser_regs *regs = &state->regs;

    if (regs->ilen < 1)
        tz_stop(FEED_ME);
    state->ofs++;
    regs->ilen--;
    *r = regs->ibuf[regs->iofs++];
    tz_continue;
}

tz_parser_result
tz_parser_peek(tz_parser_state *state, uint8_t *r)
{
    tz_parser_regs *regs = &state->regs;

    if (regs->ilen < 1)
        tz_stop(FEED_ME);
    *r = regs->ibuf[regs->iofs];
    tz_continue;
}

void
tz_parser_skip(tz_parser_state *state)
{
    tz_parser_regs *regs = &state->regs;

    regs->iofs++;
    regs->ilen--;
    state->ofs++;
}
