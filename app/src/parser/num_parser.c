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

void tz_parse_num_state_init(tz_num_parser_buffer *buffers,
                             tz_num_parser_regs *regs) {
  buffers->bytes[0] = 0;
  regs->size = 0;
  regs->sign = 0;
  regs->stop = 0;
}

tz_parser_result tz_parse_num_step(tz_num_parser_buffer *buffers,
                                   tz_num_parser_regs *regs,
                                   uint8_t b, bool natural) {
  uint8_t v, cont, s;
  cont = b >> 7;
  if (regs->size == 0 && !natural) {
    v = b & 0x3F;
    regs->sign = (b >> 6)&1;
    s = 6;
  } else {
    v = b & 0x7F;
    s = 7;
  }
  uint8_t lo = v << (regs->size & 7);
  uint8_t hi = v >> (8 - (regs->size & 7));
  int lo_idx = regs->size/8;
  int hi_idx = lo_idx+1;
  buffers->bytes[lo_idx] |= lo;
  if (hi_idx >= TZ_NUM_BUFFER_SIZE/8) {
    // accept and dismiss a few trailing zeroes
    if (hi || cont) return TZ_ERR_TOO_LARGE;
    regs->size = TZ_NUM_BUFFER_SIZE;
  } else {
    buffers->bytes[hi_idx] = hi;
    regs->size += s;
  }
  if (!cont) {
    regs->stop = true;
    tz_format_decimal(buffers->bytes, (regs->size+7)/8, buffers->decimal);
  }
  return TZ_CONTINUE;
}

tz_parser_result tz_parse_int_step(tz_num_parser_buffer *buffers,
                                   tz_num_parser_regs *regs, uint8_t b) {
  return tz_parse_num_step(buffers, regs, b, 0);
}

tz_parser_result tz_parse_nat_step(tz_num_parser_buffer *buffers,
                                   tz_num_parser_regs *regs, uint8_t b) {
  return tz_parse_num_step(buffers, regs, b, 1);
}

bool tz_adjust_decimal(const char *in,
                       int in_len,
                       char *out,
                       int out_len,
                       int nb_decimals) {
  int offset = 0;

  if (in_len <= 0 || in == NULL || out == NULL) return false;

  // drop leading non significant zeroes
  while (in_len != 1 && in[0] == '0') {
    in++;
    in_len--;
  }

  if (in_len == 1 && in[0] == '0') {
    if (out_len < 2) return false;
    out[offset++] = '0';
    out[offset++] = '\0';
    return true;
  }

  if (in_len <= nb_decimals) {
    if (out_len < 3 + nb_decimals) return false;
    int delta = nb_decimals - in_len;
    out[offset++] = '0';
    out[offset++] = '.';
    for(int i = 0; i < delta; i++) out[offset++] = '0';
    for(int i = 0; i < in_len; i++) out[offset++] = in[i];
  } else {
    if (out_len < 2 + in_len) return false;
    int delta = in_len - nb_decimals;
    for(int i = 0; i < delta; i++) out[offset++] = in[i];
    out[offset++] = '.';
    for(int i = delta; i < in_len; i++) out[offset++] = in[i];
  }

  out[offset--] = '\0';

  // drop trailing non significant zeroes
  while (out[offset] == '0') out[offset--] = '\0';
  if (out[offset] == '.') out[offset--] = '\0';

  return true;
}

bool tz_print_uint64(uint64_t value, char *out, int out_len, int nb_decimals) {
  char buff[TZ_NUM_BUFFER_SIZE] = {0};
  uint64_t power = 10;
  int offset = 0;

  while (power <= value) power *= 10;

  while (power != 1) {
    power /= 10;
    buff[offset++] = value / power + '0';
    value %= power;
  }

  buff[offset] = '\0';

  return tz_adjust_decimal(buff, offset, out, out_len, nb_decimals);
}

bool tz_string_to_uint64(const char *str, uint64_t *res) {

  if (str == NULL || res == NULL) {
    PRINTF("[ERROR] Null parameter\n");
    goto error;
  }

  memset(res, '\0', sizeof(uint64_t));
  int r = 0;
  char c;

  while ((c = *str++) != '\0') {
    if (c < '0' || c > '9') {
      PRINTF("[ERROR] Non-digit character: %c\n", c);
      goto error;
    }
    r = r * 10 + (c - '0');
    *res = *res * 10 + (c - '0');
  }

  return true;

 error:
  memset(res, '\0', sizeof(uint64_t));
  return false;
}
