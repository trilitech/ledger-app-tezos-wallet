#ifndef _TZ_NUM_PARSER_H
#define _TZ_NUM_PARSER_H	1

#include "parser_state.h"

extern void tz_parse_num_state_init (tz_num_parser_buffer *buffers,
                                     tz_num_parser_regs *regs);
extern tz_parser_result tz_parse_num_step (tz_num_parser_buffer *buffers,
                                           tz_num_parser_regs *regs,
                                           uint8_t b, bool natural, bool pretty_print);
extern tz_parser_result tz_parse_int_step (tz_num_parser_buffer *buffers,
                                           tz_num_parser_regs *regs,
                                           uint8_t b, bool pretty_print);
extern tz_parser_result tz_parse_nat_step (tz_num_parser_buffer *buffers,
                                           tz_num_parser_regs *regs,
                                           uint8_t b, bool pretty_print);

#endif /* num_state.h */
