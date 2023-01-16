#ifndef _TZ_NUM_STATE_H
#define _TZ_NUM_STATE_H	1

#include "formatting.h"

typedef struct {
  uint16_t size;
  uint8_t sign : 1, stop : 1;
} tz_num_parser_regs;

#define TZ_NUM_BUFFER_SIZE 256

typedef struct {
  uint8_t bytes[TZ_NUM_BUFFER_SIZE/8];
  char decimal[TZ_DECIMAL_BUFFER_SIZE(TZ_NUM_BUFFER_SIZE/8)];
} tz_num_parser_buffer;

#endif /* num_state.h */
