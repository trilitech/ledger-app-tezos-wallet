/* Tezos Embedded C parser for Ledger - Micheline data parser

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

/*
 * We use a few idioms throughout this file.  In each function, we
 * define a local variable "m" which is a ptr to state->micheline.
 */

#include "micheline_parser.h"
#include "num_parser.h"

/* Prototypes */

static tz_parser_result push_frame(tz_parser_state *,
                                    tz_micheline_parser_step_kind);
static tz_parser_result pop_frame(tz_parser_state *);
static tz_parser_result begin_sized(tz_parser_state *);
static tz_parser_result print_escaped(tz_parser_state *, uint8_t);
static tz_parser_result parser_put(tz_parser_state *, char);
static tz_parser_result tag_selection(tz_parser_state *, uint8_t);


#ifdef TEZOS_DEBUG
const char *const tz_micheline_parser_step_name[] = {
  "TAG",
  "PRIM_OP",
  "PRIM_NAME",
  "PRIM",
  "SIZE",
  "SEQ",
  "BYTES",
  "STRING",
  "ANNOT",
  "INT",
  "PRINT_INT",
  "CONTINUE"
};
#endif


const char hex_c[] = "0123456789ABCDEF";

void tz_micheline_parser_init(tz_parser_state *state) {
  tz_micheline_state *m = &state->micheline;

  m->frame = m->stack;
  m->stack[0].step = TZ_MICHELINE_STEP_TAG;
}

static tz_parser_result push_frame(tz_parser_state *state,
                                    tz_micheline_parser_step_kind step) {
  tz_micheline_state *m = &state->micheline;

  if (m->frame >=
      &m->stack[TZ_MICHELINE_STACK_DEPTH - 1])
    tz_raise(TOO_DEEP);
  m->frame++ ;
  m->frame->step = step;
  tz_continue;
}

static tz_parser_result pop_frame(tz_parser_state *state) {
  tz_micheline_state *m = &state->micheline;

  if (m->frame == m->stack) {
    m->frame = NULL;
    tz_stop(DONE);
  }
  m->frame--;
  tz_continue;
}

static tz_parser_result begin_sized(tz_parser_state *state) {
  tz_micheline_state *m = &state->micheline;

  if (push_frame(state, TZ_MICHELINE_STEP_SIZE)) tz_reraise;
  m->frame->step_size.size = 0;
  m->frame->stop = state->ofs+4;
  tz_continue;
}

static tz_parser_result print_escaped(tz_parser_state *state, uint8_t b) {
  char* buf = (char*) state->buffers.capture;
  tz_must(push_frame(state, TZ_MICHELINE_STEP_PRINT_CAPTURE));
  state->micheline.frame->step_capture.ofs = 0;
  switch (b) {
  case '\\': strncpy(buf,"\\\\",TZ_CAPTURE_BUFFER_SIZE); break;
  case '"': strncpy(buf,"\\\"",TZ_CAPTURE_BUFFER_SIZE); break;
  case '\r': strncpy(buf,"\\r",TZ_CAPTURE_BUFFER_SIZE); break;
  case '\n': strncpy(buf,"\\n",TZ_CAPTURE_BUFFER_SIZE); break;
  case '\t': strncpy(buf,"\\t",TZ_CAPTURE_BUFFER_SIZE); break;
  default:
    buf[0] = '0' + b/100;
    buf[1] = '0' + (b/10)%10;
    buf[2] = '0' + b%10;
    buf[3] = 0;
    break;
  }
  tz_continue;
}

static tz_parser_result parser_put(tz_parser_state *state, char c) {
  PRINTF("[DEBUG] put(char: '%c',int: %d)\n", c, (int) c);
  return (tz_parser_put(state, c));
}

static tz_parser_result tag_selection(tz_parser_state *state, uint8_t t) {
  tz_micheline_state *m = &state->micheline;

  switch (t) {
  case TZ_MICHELINE_TAG_INT: {
    m->frame->step = TZ_MICHELINE_STEP_INT;
    tz_parse_num_state_init(&state->buffers.num,
                            &m->frame->step_int);
    for (int i = 0; i < TZ_NUM_BUFFER_SIZE/8;i++)
      state->buffers.num.bytes[i] = 0;
    break;
  }
  case TZ_MICHELINE_TAG_SEQ: {
    m->frame->step = TZ_MICHELINE_STEP_SEQ;
    m->frame->step_seq.first = true;
    tz_must(begin_sized(state));
    break;
  }
  case TZ_MICHELINE_TAG_BYTES: {
    m->frame->step = TZ_MICHELINE_STEP_BYTES;
    m->frame->step_bytes.first = true;
    m->frame->step_bytes.has_rem_half = false;
    tz_must(begin_sized(state));
    break;
  }
  case TZ_MICHELINE_TAG_STRING: {
    m->frame->step = TZ_MICHELINE_STEP_STRING;
    m->frame->step_string.first = true;
    tz_must(begin_sized(state));
    break;
  }
  case TZ_MICHELINE_TAG_PRIM_0_ANNOTS:
  case TZ_MICHELINE_TAG_PRIM_0_NOANNOTS:
  case TZ_MICHELINE_TAG_PRIM_1_ANNOTS:
  case TZ_MICHELINE_TAG_PRIM_1_NOANNOTS:
  case TZ_MICHELINE_TAG_PRIM_2_ANNOTS:
  case TZ_MICHELINE_TAG_PRIM_2_NOANNOTS: {
    uint8_t nargs = (t-3) >> 1;
    uint8_t annot = (~t & 1);
    uint8_t wrap = (m->frame > m->stack)
      && m->frame[-1].step == TZ_MICHELINE_STEP_PRIM
      && (nargs>0 || annot);
    goto common_prim;
    case TZ_MICHELINE_TAG_PRIM_N:
      wrap = (m->frame > m->stack) &&
              m->frame[-1].step == TZ_MICHELINE_STEP_PRIM;
      nargs = 3;
      annot = true;
    common_prim:
      m->frame->step = TZ_MICHELINE_STEP_PRIM_OP;
      m->frame->step_prim.ofs = 0;
      m->frame->step_prim.nargs = nargs;
      m->frame->step_prim.wrap = wrap;
      m->frame->step_prim.spc = false;
      m->frame->step_prim.first = true;
      m->frame->step_prim.annot = annot;
      break;
  }
  default: tz_raise(INVALID_TAG);
  }
  tz_continue;
}

tz_parser_result tz_micheline_parser_step(tz_parser_state *state) {
  tz_micheline_state *m = &state->micheline;

  // cannot restart after error
  if (TZ_IS_ERR(state->errno)) tz_reraise;
  // nothing else to do
  if (state->micheline.frame == NULL) tz_stop(DONE);

  PRINTF("[DEBUG] micheline(frame: %d, offset:%d/%d, step: %s, errno: %s)\n",
         (int) (m->frame - m->stack),
         (int) state->ofs,
         (int) m->frame->stop,
         (const char*) PIC(tz_micheline_parser_step_name[m->frame->step]),
         tz_parser_result_name(state->errno));

  switch (state->micheline.frame->step) {
  case TZ_MICHELINE_STEP_INT: {
    uint8_t b;
    tz_must(tz_parser_read(state, &b));
    tz_must(tz_parse_int_step(&state->buffers.num, &m->frame->step_int, b));
    if (m->frame->step_int.stop) {
      m->frame->step = TZ_MICHELINE_STEP_PRINT_INT;
      m->frame->step_int.size = 0;
    }
    break;
  }
  case TZ_MICHELINE_STEP_PRINT_INT: {
    if (m->frame->step_int.sign) {
      tz_must(parser_put(state, '-'));
      m->frame->step_int.sign = 0;
    } else if (state->buffers.num.decimal[m->frame->step_int.size]) {
      tz_must(parser_put(state, state->buffers.num.decimal[m->frame->step_int.size]));
      m->frame->step_int.size++;
    } else {
      tz_must(pop_frame(state));
    }
    break;
  }
  case TZ_MICHELINE_STEP_SIZE: {
    uint8_t b;
    tz_must(tz_parser_read(state, &b));
    if (m->frame->step_size.size > 255) tz_raise(TOO_LARGE); // enforce 16-bit restriction
    m->frame->step_size.size = m->frame->step_size.size << 8 | b;
    if (m->frame->stop == state->ofs) {
      m->frame[-1].stop = state->ofs + m->frame->step_size.size;
      tz_must(pop_frame(state));
    }
    break;
  }
  case TZ_MICHELINE_STEP_SEQ: {
    if (m->frame->stop == state->ofs) {
      if (m->frame->step_seq.first) {
        tz_must(parser_put(state, '{'));
        m->frame->step_seq.first = false;
      } else {
        tz_must(parser_put(state, '}'));
        tz_must(pop_frame(state));
      }
    } else {
      if (m->frame->step_seq.first) {
        tz_must(parser_put(state, '{'));
        m->frame->step_seq.first = false;
      } else {
        tz_must(parser_put(state, ';'));
      }
      tz_must(push_frame(state, TZ_MICHELINE_STEP_TAG));
    }
    break;
  }
  case TZ_MICHELINE_STEP_PRINT_CAPTURE: {
    if (state->buffers.capture[state->micheline.frame->step_capture.ofs]) {
      tz_must(parser_put(state, state->buffers.capture[m->frame->step_capture.ofs]));
      m->frame->step_capture.ofs++;
    } else {
      tz_must(pop_frame(state));
    }
    break;
  }
  case TZ_MICHELINE_STEP_BYTES: {
    if (m->frame->step_bytes.has_rem_half) {
      tz_must(parser_put(state, m->frame->step_bytes.rem_half));
      m->frame->step_bytes.has_rem_half = 0;
    } else if (state->micheline.frame->step_bytes.first) {
      tz_must(parser_put(state, '0'));
      m->frame->step_bytes.has_rem_half = true;
      m->frame->step_bytes.rem_half = 'x';
      m->frame->step_bytes.first = false;
    } else if (m->frame->stop == state->ofs) {
      tz_must(pop_frame(state));
    } else {
      uint8_t b;
      char half;
      tz_must(tz_parser_peek(state, &b));
      half = hex_c[(b & 0xF0) >> 4];
      tz_must(parser_put(state, half));
      m->frame->step_bytes.has_rem_half = true;
      m->frame->step_bytes.rem_half = hex_c[b & 0x0F];
      tz_parser_skip(state);
    }
    break;
  }
  case TZ_MICHELINE_STEP_STRING: {
    if (m->frame->step_string.first) {
      tz_must(parser_put(state, '\"'));
      m->frame->step_string.first = false;
    } else if (m->frame->stop == state->ofs) {
      tz_must(parser_put(state, '\"'));
      tz_must(pop_frame(state));
    } else {
      uint8_t b;
      tz_must(tz_parser_peek(state, &b));
      if (b >= 0x20 && b < 0x80 && b != '\"' && b != '\\') {
        tz_must(parser_put(state, b));
        tz_parser_skip(state);
      } else {
        tz_parser_skip(state);
        tz_must(print_escaped(state, b));
      }
    }
    break;
  }
  case TZ_MICHELINE_STEP_ANNOT: {
    if (m->frame->step_annot.first) {
      // after reading the size, copy the stop in parent TZ_MICHELINE_STEP_PRIM frame
      m->frame[-1].stop = m->frame->stop;
    }
    if (m->frame->stop == state->ofs) {
      tz_must(pop_frame(state));
    } else {
      if (m->frame->step_annot.first) {
        tz_must(parser_put(state, ' '));
        m->frame->step_annot.first = false;
      }
      uint8_t b;
      tz_must(tz_parser_peek(state, &b));
      tz_must(parser_put(state, b));
      tz_parser_skip(state);
    }
    break;
  }
  case TZ_MICHELINE_STEP_PRIM_OP: {
    uint8_t op;
    tz_must(tz_parser_read(state, &op));
    if (tz_michelson_op_name(op) == NULL) tz_raise(INVALID_OP);
    m->frame->step = TZ_MICHELINE_STEP_PRIM_NAME;
    m->frame->step_prim.op = op;
    break;
  }
  case TZ_MICHELINE_STEP_PRIM_NAME: {
    if (m->frame->step_prim.wrap && m->frame->step_prim.first) {
      tz_must(parser_put(state, '('));
      m->frame->step_prim.first = false;
    }
    if (tz_michelson_op_name(m->frame->step_prim.op)[m->frame->step_prim.ofs]) {
      tz_must(parser_put(state, tz_michelson_op_name(m->frame->step_prim.op)[m->frame->step_prim.ofs]));
      m->frame->step_prim.ofs++;
    } else {
      m->frame->step = TZ_MICHELINE_STEP_PRIM;
      if (m->frame->step_prim.nargs == 3)
        tz_must(begin_sized(state));
    }
    break;
  }
  case TZ_MICHELINE_STEP_PRIM: {
    if (m->frame->step_prim.nargs == 0 || (m->frame->step_prim.nargs == 3 && m->frame->stop == state->ofs)) {
      if (m->frame->step_prim.annot) {
        m->frame->step_prim.annot = false;
        tz_must(push_frame(state, TZ_MICHELINE_STEP_ANNOT));
        m->frame->step_annot.first = true;
        tz_must(begin_sized(state));
      } else {
        if (m->frame->step_prim.wrap)
          tz_must(parser_put(state, ')'));
        tz_must(pop_frame(state));
      }
    } else if (!m->frame->step_prim.spc) {
      tz_must(parser_put(state, ' '));
      m->frame->step_prim.spc = true;
    } else {
      if (m->frame->step_prim.nargs < 3)
        m->frame->step_prim.nargs--;
      m->frame->step_prim.spc = false;
      tz_must(push_frame(state, TZ_MICHELINE_STEP_TAG));
    }
    break;
  }
  case TZ_MICHELINE_STEP_TAG: {
    uint8_t t;
    tz_must(tz_parser_read(state, &t));
    tz_must(tag_selection(state, t));
    break;
    default: tz_raise(INVALID_STATE);
  }
  }
  tz_continue;
}
