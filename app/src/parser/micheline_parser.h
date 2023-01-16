#ifndef _TZ_MICHELINE_PARSER_H
#define _TZ_MICHELINE_PARSER_H	1

#include "parser_state.h"

extern void tz_micheline_parser_init(tz_parser_state *state, const uint8_t *pat, size_t pat_len);
extern tz_parser_result tz_micheline_parser_step(tz_parser_state *state, tz_parser_regs *regs);

#endif /* micheline_parser.h */
