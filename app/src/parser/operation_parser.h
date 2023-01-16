#ifndef _TZ_OPERATION_PARSER_H
#define _TZ_OPERATION_PARSER_H	1

#include "parser_state.h"

#define TZ_UNKNOWN_SIZE 0xFFFF
extern void tz_operation_parser_init(tz_parser_state *state, uint16_t size, bool skip_magic);
extern void tz_operation_parser_set_size(tz_parser_state *state, uint16_t size);
extern tz_parser_result tz_operation_parser_step(tz_parser_state *state, tz_parser_regs *regs);

#endif /* operation_parser.h */
