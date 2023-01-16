#ifndef _TZ_PARSER_STATE_H
#define _TZ_PARSER_STATE_H	1

#include "num_state.h"
#include "micheline_state.h"
#include "operation_state.h"

// Parser buffers and buffer handling registers

typedef struct {
  // update `ibuf`, `iofs` and `ilen` at once with `parser_regs_refill`
  // invariant between two refills: `iofs + ilen` = (constant) number of readable bytes in `ibuf`
  uint8_t* ibuf; // input buffer
  size_t iofs; // current offset
  size_t ilen; // remaining bytes readable in input
  // update `obuf`, `oofs` and `olen` at once with `parser_regs_flush`
  // invariant between two refills: `oofs + olen` = (constant) number of readable bytes in `obuf`
  char* obuf; // output buffer
  size_t oofs; // current offset
  size_t olen; // remaining bytes writable in output
} tz_parser_regs;
extern void tz_parser_regs_flush(tz_parser_regs *regs, char *obuf, size_t olen);
extern void tz_parser_regs_refill(tz_parser_regs *regs, uint8_t *ibuf, size_t ilen);

// Parser state

#define TZ_FIELD_NAME_SIZE 80
#define TZ_CAPTURE_BUFFER_SIZE 256

typedef enum {
  // success and non blocking, should loop again
  TZ_CONTINUE = 0, // fall through rest of current step
  TZ_BREAK, // signals caller to return, errno should be CONTINUE
  // success but parsing blocked
  TZ_BLO_DONE = 100, // parsing complete
  TZ_BLO_FEED_ME, // blocked on read from input
  TZ_BLO_IM_FULL, // blocked on output space
  // everything below is an error
  TZ_ERR_INVALID_TAG = 200,
  TZ_ERR_INVALID_OP,
  TZ_ERR_UNSUPPORTED,
  TZ_ERR_TOO_LARGE,
  TZ_ERR_MISMATCH,
  TZ_ERR_TOO_DEEP,
  TZ_ERR_INVALID_STATE,
  TZ_ERR_BAD_PATTERN
} tz_parser_result;
#define TZ_IS_BLOCKED(code) (code >= 100)
#define TZ_IS_ERR(code) (code >= 200)

#ifdef TEZOS_DEBUG
const char* tz_parser_result_name(tz_parser_result code);
#endif

typedef struct {
  // common fields to communicate with caller
  tz_parser_result errno;
  char field_name[TZ_FIELD_NAME_SIZE];
  // common singleton buffers
  int ofs;
  struct {
    tz_num_parser_buffer num;
    uint8_t capture[TZ_CAPTURE_BUFFER_SIZE];
  } buffers;
  // input type specific state
  tz_micheline_state micheline;
  tz_operation_state operation;
} tz_parser_state;

extern void tz_parser_init(tz_parser_state *state);

extern tz_parser_result tz_parser_put(tz_parser_state *state, tz_parser_regs *regs, char c);
extern tz_parser_result tz_parser_read(tz_parser_state *state, tz_parser_regs *regs, uint8_t *r);
extern tz_parser_result tz_parser_peek(tz_parser_state *state, tz_parser_regs *regs,uint8_t *r);
extern void tz_parser_skip(tz_parser_state *state, tz_parser_regs *regs);

// error handling utils

extern tz_parser_result tz_parser_set_errno (tz_parser_state *state, tz_parser_result code);
#ifdef TEZOS_DEBUG
#  define tz_return(e) do { \
    tz_parser_result c = (e); \
    if (c) PRINTF ("[DEBUG] tz_return(code: %s, loc: %s:%d)\n", \
                   tz_parser_result_name (c), __FILE__, __LINE__);      \
    return tz_parser_set_errno (state, c); \
  } while (0)
#else
#  define tz_return(e) return tz_parser_set_errno (state, e)
#endif
#define tz_raise(e) tz_return (TZ_ERR_##e)
#define tz_stop(e) tz_return (TZ_BLO_##e)
#define tz_reraise return state->errno
#define tz_must(cond) do { tz_parser_result errno = cond; if(errno) tz_return(errno); } while (0)
#define tz_continue tz_return (TZ_CONTINUE)
#define tz_break tz_return (TZ_BREAK)

#endif /* parser_state.h */
