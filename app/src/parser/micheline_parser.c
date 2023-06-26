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

#include "micheline_parser.h"
#include "num_parser.h"

/* Prototypes */

static tz_parser_result push_frame(tz_parser_state *,
                                    tz_micheline_parser_step_kind);
static tz_parser_result pop_frame(tz_parser_state *);
static tz_parser_result begin_sized(tz_parser_state *);
static tz_parser_result print_escaped(tz_parser_state *, uint8_t);
static tz_parser_result parser_put(tz_parser_state *, tz_parser_regs *, char);
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
  state->micheline.frame = state->micheline.stack;
  state->micheline.stack[0].step = TZ_MICHELINE_STEP_TAG;
}

static tz_parser_result push_frame (tz_parser_state *state, tz_micheline_parser_step_kind step) {
  if (state->micheline.frame >= &state->micheline.stack[TZ_MICHELINE_STACK_DEPTH - 1])
    tz_raise (TOO_DEEP);
  state->micheline.frame++ ;
  state->micheline.frame->step = step;
  tz_continue;
}

static tz_parser_result pop_frame (tz_parser_state *state) {
  if (state->micheline.frame == state->micheline.stack) {
    state->micheline.frame = NULL;
    tz_stop (DONE);
  }
  state->micheline.frame--;
  tz_continue;
}

static tz_parser_result begin_sized (tz_parser_state *state) {
  if (push_frame (state, TZ_MICHELINE_STEP_SIZE)) tz_reraise;
  state->micheline.frame->step_size.size = 0;
  state->micheline.frame->stop = state->ofs+4;
  tz_continue;
}

static tz_parser_result print_escaped(tz_parser_state *state, uint8_t b) {
  char* buf = (char*) state->buffers.capture;
  tz_must (push_frame (state, TZ_MICHELINE_STEP_PRINT_CAPTURE));
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

static tz_parser_result parser_put(tz_parser_state *state, tz_parser_regs *regs, char c) {
  PRINTF("[DEBUG] put(char: '%c',int: %d)\n", c, (int) c);
  return (tz_parser_put (state, regs, c));
}

static tz_parser_result tag_selection (tz_parser_state *state, uint8_t t) {
  switch (t) {
  case TZ_MICHELINE_TAG_INT: {
    state->micheline.frame->step = TZ_MICHELINE_STEP_INT;
    tz_parse_num_state_init(&state->buffers.num, &state->micheline.frame->step_int);
    for(int i = 0; i < TZ_NUM_BUFFER_SIZE/8;i++) state->buffers.num.bytes[i] = 0;
    break;
  }
  case TZ_MICHELINE_TAG_SEQ: {
    state->micheline.frame->step = TZ_MICHELINE_STEP_SEQ;
    state->micheline.frame->step_seq.first = true;
    tz_must (begin_sized (state));
    break;
  }
  case TZ_MICHELINE_TAG_BYTES: {
    state->micheline.frame->step = TZ_MICHELINE_STEP_BYTES;
    state->micheline.frame->step_bytes.first = true;
    state->micheline.frame->step_bytes.has_rem_half = false;
    tz_must (begin_sized (state));
    break;
  }
  case TZ_MICHELINE_TAG_STRING: {
    state->micheline.frame->step = TZ_MICHELINE_STEP_STRING;
    state->micheline.frame->step_string.first = true;
    tz_must (begin_sized (state));
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
    uint8_t wrap = (state->micheline.frame > state->micheline.stack)
      && state->micheline.frame[-1].step == TZ_MICHELINE_STEP_PRIM
      && (nargs>0 || annot);
    goto common_prim;
    case TZ_MICHELINE_TAG_PRIM_N:
      wrap = (state->micheline.frame > state->micheline.stack) && state->micheline.frame[-1].step == TZ_MICHELINE_STEP_PRIM;
      nargs = 3;
      annot = true;
    common_prim:
      state->micheline.frame->step = TZ_MICHELINE_STEP_PRIM_OP;
      state->micheline.frame->step_prim.ofs = 0;
      state->micheline.frame->step_prim.nargs = nargs;
      state->micheline.frame->step_prim.wrap = wrap;
      state->micheline.frame->step_prim.spc = false;
      state->micheline.frame->step_prim.first = true;
      state->micheline.frame->step_prim.annot = annot;
      break;
  }
  default: tz_raise (INVALID_TAG);
  }
  tz_continue;
}

tz_parser_result tz_micheline_parser_step(tz_parser_state *state, tz_parser_regs *regs) {
  // cannot restart after error
  if (TZ_IS_ERR(state->errno)) tz_reraise;
  // nothing else to do
  if (state->micheline.frame == NULL) tz_stop (DONE);

  PRINTF("[DEBUG] micheline(frame: %d, offset:%d/%d, step: %s, errno: %s)\n",
         (int) (state->micheline.frame - state->micheline.stack),
         (int) state->ofs,
         (int) state->micheline.frame->stop,
         (const char*) PIC(tz_micheline_parser_step_name[state->micheline.frame->step]),
         tz_parser_result_name(state->errno));

  switch (state->micheline.frame->step) {
  case TZ_MICHELINE_STEP_INT: {
    uint8_t b;
    tz_must (tz_parser_read(state, regs,&b));
    tz_must (tz_parse_int_step (&state->buffers.num, &state->micheline.frame->step_int, b));
    if (state->micheline.frame->step_int.stop) {
      state->micheline.frame->step = TZ_MICHELINE_STEP_PRINT_INT;
      state->micheline.frame->step_int.size = 0;
    }
    break;
  }
  case TZ_MICHELINE_STEP_PRINT_INT: {
    if (state->micheline.frame->step_int.sign) {
      tz_must (parser_put(state, regs, '-'));
      state->micheline.frame->step_int.sign = 0;
    } else if (state->buffers.num.decimal[state->micheline.frame->step_int.size]) {
      tz_must (parser_put(state, regs, state->buffers.num.decimal[state->micheline.frame->step_int.size]));
      state->micheline.frame->step_int.size++;
    } else {
      tz_must (pop_frame (state));
    }
    break;
  }
  case TZ_MICHELINE_STEP_SIZE: {
    uint8_t b;
    tz_must (tz_parser_read(state, regs,&b));
    if (state->micheline.frame->step_size.size > 255) tz_raise (TOO_LARGE); // enforce 16-bit restriction
    state->micheline.frame->step_size.size = state->micheline.frame->step_size.size << 8 | b;
    if (state->micheline.frame->stop == state->ofs) {
      state->micheline.frame[-1].stop = state->ofs + state->micheline.frame->step_size.size;
      tz_must (pop_frame (state));
    }
    break;
  }
  case TZ_MICHELINE_STEP_SEQ: {
    if (state->micheline.frame->stop == state->ofs) {
      if (state->micheline.frame->step_seq.first) {
        tz_must (parser_put(state, regs, '{'));
        state->micheline.frame->step_seq.first = false;
      } else {
        tz_must (parser_put(state, regs, '}'));
        tz_must (pop_frame (state));
      }
    } else {
      if (state->micheline.frame->step_seq.first) {
        tz_must (parser_put(state, regs, '{'));
        state->micheline.frame->step_seq.first = false;
      } else {
        tz_must (parser_put(state, regs, ';'));
      }
      tz_must (push_frame (state, TZ_MICHELINE_STEP_TAG));
    }
    break;
  }
  case TZ_MICHELINE_STEP_PRINT_CAPTURE: {
    if (state->buffers.capture[state->micheline.frame->step_capture.ofs]) {
      tz_must (parser_put(state, regs, state->buffers.capture[state->micheline.frame->step_capture.ofs]));
      state->micheline.frame->step_capture.ofs++;
    } else {
      tz_must (pop_frame (state));
    }
    break;
  }
  case TZ_MICHELINE_STEP_BYTES: {
    if (state->micheline.frame->step_bytes.has_rem_half) {
      tz_must (parser_put(state, regs, state->micheline.frame->step_bytes.rem_half));
      state->micheline.frame->step_bytes.has_rem_half = 0;
    } else if (state->micheline.frame->step_bytes.first) {
      tz_must (parser_put(state, regs, '0'));
      state->micheline.frame->step_bytes.has_rem_half = true;
      state->micheline.frame->step_bytes.rem_half = 'x';
      state->micheline.frame->step_bytes.first = false;
    } else if (state->micheline.frame->stop == state->ofs) {
      tz_must (pop_frame (state));
    } else {
      uint8_t b;
      char half;
      tz_must (tz_parser_peek(state,regs,&b));
      half = hex_c[(b & 0xF0) >> 4];
      tz_must (parser_put(state, regs, half));
      state->micheline.frame->step_bytes.has_rem_half = true;
      state->micheline.frame->step_bytes.rem_half = hex_c[b & 0x0F];
      tz_parser_skip(state, regs);
    }
    break;
  }
  case TZ_MICHELINE_STEP_STRING: {
    if (state->micheline.frame->step_string.first) {
      tz_must (parser_put(state, regs, '\"'));
      state->micheline.frame->step_string.first = false;
    } else if (state->micheline.frame->stop == state->ofs) {
      tz_must (parser_put(state, regs, '\"'));
      tz_must (pop_frame (state));
    } else {
      uint8_t b;
      tz_must (tz_parser_peek(state,regs,&b));
      if (b >= 0x20 && b < 0x80 && b != '\"' && b != '\\') {
        tz_must (parser_put(state, regs, b));
        tz_parser_skip(state, regs);
      } else {
        tz_parser_skip(state, regs);
        tz_must (print_escaped (state, b));
      }
    }
    break;
  }
  case TZ_MICHELINE_STEP_ANNOT: {
    if (state->micheline.frame->step_annot.first) {
      // after reading the size, copy the stop in parent TZ_MICHELINE_STEP_PRIM frame
      state->micheline.frame[-1].stop = state->micheline.frame->stop;
    }
    if (state->micheline.frame->stop == state->ofs) {
      tz_must (pop_frame (state));
    } else {
      if (state->micheline.frame->step_annot.first) {
        tz_must (parser_put(state, regs, ' '));
        state->micheline.frame->step_annot.first = false;
      }
      uint8_t b;
      tz_must (tz_parser_peek(state,regs,&b));
      tz_must (parser_put(state, regs, b));
      tz_parser_skip(state, regs);
    }
    break;
  }
  case TZ_MICHELINE_STEP_PRIM_OP: {
    uint8_t op;
    tz_must (tz_parser_read(state, regs,&op));
    if (tz_michelson_op_name(op) == NULL) tz_raise (INVALID_OP);
    state->micheline.frame->step = TZ_MICHELINE_STEP_PRIM_NAME;
    state->micheline.frame->step_prim.op = op;
    break;
  }
  case TZ_MICHELINE_STEP_PRIM_NAME: {
    if (state->micheline.frame->step_prim.wrap && state->micheline.frame->step_prim.first) {
      tz_must (parser_put(state, regs, '('));
      state->micheline.frame->step_prim.first = false;
    }
    if (tz_michelson_op_name(state->micheline.frame->step_prim.op)[state->micheline.frame->step_prim.ofs]) {
      tz_must (parser_put(state, regs, tz_michelson_op_name(state->micheline.frame->step_prim.op)[state->micheline.frame->step_prim.ofs]));
      state->micheline.frame->step_prim.ofs++;
    } else {
      state->micheline.frame->step = TZ_MICHELINE_STEP_PRIM;
      if (state->micheline.frame->step_prim.nargs == 3)
        tz_must (begin_sized (state));
    }
    break;
  }
  case TZ_MICHELINE_STEP_PRIM: {
    if (state->micheline.frame->step_prim.nargs == 0 || (state->micheline.frame->step_prim.nargs == 3 && state->micheline.frame->stop == state->ofs)) {
      if (state->micheline.frame->step_prim.annot) {
        state->micheline.frame->step_prim.annot = false;
        tz_must (push_frame (state, TZ_MICHELINE_STEP_ANNOT));
        state->micheline.frame->step_annot.first = true;
        tz_must (begin_sized (state));
      } else {
        if (state->micheline.frame->step_prim.wrap)
          tz_must (parser_put(state, regs, ')'));
        tz_must (pop_frame (state));
      }
    } else if (!state->micheline.frame->step_prim.spc) {
      tz_must (parser_put(state, regs, ' '));
      state->micheline.frame->step_prim.spc = true;
    } else {
      if (state->micheline.frame->step_prim.nargs < 3) state->micheline.frame->step_prim.nargs--;
      state->micheline.frame->step_prim.spc = false;
      tz_must (push_frame (state, TZ_MICHELINE_STEP_TAG));
    }
    break;
  }
  case TZ_MICHELINE_STEP_TAG: {
    uint8_t t;
    tz_must (tz_parser_read(state,regs,&t));
    tz_must (tag_selection(state, t));
    break;
    default: tz_raise (INVALID_STATE);
  }
  }
  tz_continue;
}
