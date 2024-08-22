/* Tezos Embedded C parser for Ledger - Big num parser

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include "num_parser.h"

void
tz_parse_num_state_init(tz_num_parser_buffer *buffers,
                        tz_num_parser_regs   *regs)
{
    buffers->bytes[0] = 0;
    regs->size        = 0;
    regs->sign        = 0;
    regs->stop        = 0;
}

tz_parser_result
tz_parse_num_step(tz_num_parser_buffer *buffers, tz_num_parser_regs *regs,
                  uint8_t b, bool natural)
{
    uint8_t v, cont, s;
    cont = b >> 7;
    if ((regs->size == 0) && !natural) {
        v          = b & 0x3F;
        regs->sign = (b >> 6) & 1;
        s          = 6;
    } else {
        v = b & 0x7F;
        s = 7;
    }
    uint8_t lo     = v << (regs->size & 7);
    uint8_t hi     = v >> (8 - (regs->size & 7));
    int     lo_idx = regs->size / 8;
    int     hi_idx = lo_idx + 1;
    buffers->bytes[lo_idx] |= lo;
    if (hi_idx >= (TZ_NUM_BUFFER_SIZE / 8)) {
        // accept and dismiss a few trailing zeroes
        if (hi || cont) {
            return TZ_ERR_TOO_LARGE;
        }
        regs->size = TZ_NUM_BUFFER_SIZE;
    } else {
        buffers->bytes[hi_idx] = hi;
        regs->size += s;
    }
    if (!cont) {
        regs->stop = true;
        tz_format_decimal(buffers->bytes, (regs->size + 7) / 8,
                          buffers->decimal, sizeof(buffers->decimal));
    }
    return TZ_CONTINUE;
}

tz_parser_result
tz_parse_int_step(tz_num_parser_buffer *buffers, tz_num_parser_regs *regs,
                  uint8_t b)
{
    return tz_parse_num_step(buffers, regs, b, 0);
}

tz_parser_result
tz_parse_nat_step(tz_num_parser_buffer *buffers, tz_num_parser_regs *regs,
                  uint8_t b)
{
    return tz_parse_num_step(buffers, regs, b, 1);
}

bool
tz_string_to_mutez(const char *str, uint64_t *res)
{
    if ((str == NULL) || (res == NULL)) {
        PRINTF("[ERROR] Null parameter\n");
        return false;
    }

    *res = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if ((str[i] < '0') || (str[i] > '9')) {
            PRINTF("[ERROR] Non-digit character: %c\n", str[i]);
            return false;
        }
        *res = (*res * 10) + (str[i] - '0');
    }

    return true;
}

bool
tz_mutez_to_string(char *str, uint64_t microtez)
{
    if (str == NULL) {
        return false;
    }

    uint64_t integer_part = microtez / 1000000;
    uint64_t decimal_part = microtez % 1000000;

    // Convert integer part to string
    size_t i = 0;
    if (integer_part == 0) {
        str[i++] = '0';
    } else {
        uint64_t temp    = integer_part;
        size_t   int_len = 0;
        while (temp > 0) {
            temp /= 10;
            int_len++;
        }
        for (size_t j = int_len; j > 0; j--) {
            str[i + j - 1] = '0' + (integer_part % 10);
            integer_part /= 10;
        }
        i += int_len;
    }

    // Add decimal point
    str[i++] = '.';

    // Convert decimal part to string
    size_t decimal_start = i;
    for (size_t j = 0; j < 6; j++) {
        str[i++]     = '0' + (decimal_part / 100000);
        decimal_part = (decimal_part % 100000) * 10;
    }

    // Remove trailing zeros from the decimal part
    while (i > decimal_start && str[i - 1] == '0') {
        i--;
    }

    // Remove decimal point if no decimal part
    if (i > decimal_start && str[i - 1] == '.') {
        i--;
    }

    // Append " XTZ"
    const char *xtz = " XTZ";
    for (size_t j = 0; xtz[j] != '\0'; j++) {
        str[i++] = xtz[j];
    }

    // Null-terminate the string
    str[i] = '\0';

    return true;
}
