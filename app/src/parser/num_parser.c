#include "num_parser.h"

void tz_parse_num_state_init (tz_num_parser_buffer *buffers, tz_num_parser_regs *regs) {
  buffers->bytes[0] = 0;
  regs->size = 0;
  regs->sign = 0;
  regs->stop = 0;
}

tz_parser_result tz_parse_num_step (tz_num_parser_buffer *buffers, tz_num_parser_regs *regs,
                                    uint8_t b, bool natural, bool pretty_print) {
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
  if(!cont) {
    regs->stop = true;
    if (pretty_print)
      tz_format_decimal (buffers->bytes, (regs->size+7)/8, buffers->decimal);
  }
  return TZ_CONTINUE;
}

tz_parser_result tz_parse_int_step (tz_num_parser_buffer *buffers, tz_num_parser_regs *regs,
                                    uint8_t b, bool pretty_print) {
  return tz_parse_num_step (buffers, regs, b, 0, pretty_print);
}

tz_parser_result tz_parse_nat_step (tz_num_parser_buffer *buffers, tz_num_parser_regs *regs,
                                    uint8_t b, bool pretty_print) {
  return tz_parse_num_step (buffers, regs, b, 1, pretty_print);
}
