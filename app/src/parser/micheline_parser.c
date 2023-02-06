#include "micheline_parser.h"
#include "num_parser.h"

#ifdef TEZOS_DEBUG
const char *const tz_micheline_parser_step_name[] = {
  "TAG",
  "PRIM_OP",
  "PRIM_NAME",
  "PRIM",
  "SIZE",
  "SEQ",
  "CAPTURING",
  "LISTING",
  "BRANCHING",
  "BYTES",
  "STRING",
  "ANNOT",
  "INT",
  "PRINT_INT",
  "CAPTURE_BYTES",
  "PRINT_CAPTURE"
};
#endif


const char hex_c[] = "0123456789ABCDEF";

const uint8_t default_pattern[] = {
  TZ_MICHELINE_TAG_PRIM_2_NOANNOTS,
  TZ_MICHELSON_OP_sapling_transaction_deprecated,
  TZ_MICHELINE_TAG_INT,
  TZ_CAP_STREAM_ANY,
  TZ_MICHELINE_TAG_STRING,
  0, 0, 0, 4,        // size of field name (BE)
  'D', 'a', 't', 'a' // field name
};

void tz_micheline_parser_init(tz_parser_state *state, const uint8_t *pat, size_t pat_len) {
  if (pat == NULL) {
    pat = default_pattern;
    pat_len = sizeof(default_pattern);
  }
  state->micheline.frame = state->micheline.stack;
  state->micheline.stack[0].step = TZ_MICHELINE_STEP_TAG;
  state->micheline.pat = pat;
  state->micheline.pat_len = pat_len;
  state->micheline.pat_ofs = 0;
  state->micheline.capturing = false;
}

static int read_pat(tz_parser_state *state, uint8_t *r) {
  if ((unsigned) state->micheline.pat_ofs >= state->micheline.pat_len) return 1;
  *r = state->micheline.pat[state->micheline.pat_ofs];
  state->micheline.pat_ofs++;
  return 0;
}

static int skip_pat(tz_parser_state *state, uint8_t e) {
  if (state->micheline.capturing) return 0;
  if ((unsigned) state->micheline.pat_ofs >= state->micheline.pat_len) return 1;
  if (state->micheline.pat[state->micheline.pat_ofs] != e) return 1;
  state->micheline.pat_ofs++;
  return 0;
}

static tz_parser_result size_pat(tz_parser_state *state, uint16_t *r) {
  uint8_t b0, b1, b2, b3;
  if (read_pat(state, &b3)) tz_raise (BAD_PATTERN);
  if (read_pat(state, &b2)) tz_raise (BAD_PATTERN);
  if (read_pat(state, &b1)) tz_raise (BAD_PATTERN);
  if (read_pat(state, &b0)) tz_raise (BAD_PATTERN);
  if (b2 || b3) tz_raise (BAD_PATTERN); // enforce 16-bit restriction
  *r = (b0 | (b1 << 8));
  tz_continue;
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
  if (!state->micheline.capturing) {
    tz_must (size_pat(state, &state->micheline.frame->pat_stop));
    state->micheline.frame->pat_stop += state->micheline.pat_ofs;
  }
  if (push_frame (state, TZ_MICHELINE_STEP_SIZE)) tz_reraise;
  state->micheline.frame->step_size.size = 0;
  state->micheline.frame->stop = state->ofs+4;
  tz_continue;
}

static tz_parser_result continue_sized (tz_parser_state *state) {
  if (!state->micheline.capturing)
    if (state->micheline.frame->pat_stop == state->micheline.pat_ofs)
      tz_raise (MISMATCH);
  tz_continue;
}

static tz_parser_result end_sized (tz_parser_state *state) {
  if (!state->micheline.capturing)
    if (state->micheline.frame->pat_stop != state->micheline.pat_ofs)
      tz_raise (MISMATCH);
  tz_continue;
}

static tz_parser_result may_start_capture(tz_parser_state *state, uint8_t t) {
  // abort if already capturing
  if (state->micheline.capturing)
    tz_continue;
  // match magic pattern start (TZ_MICHELINE_TAG_PRIM_2_NOANNOTS + sapling_transaction_deprecated)
  if ((unsigned) state->micheline.pat_ofs >= state->micheline.pat_len)
    tz_raise (BAD_PATTERN);
  if (state->micheline.pat[state->micheline.pat_ofs] != TZ_MICHELINE_TAG_PRIM_2_NOANNOTS) {
    if (skip_pat(state, t)) tz_raise (MISMATCH);
    tz_continue;
  }
  if ((unsigned) state->micheline.pat_ofs + 1 >= state->micheline.pat_len)
    tz_raise (BAD_PATTERN);
  if ((uint8_t) state->micheline.pat[state->micheline.pat_ofs + 1]
      != (uint8_t) TZ_MICHELSON_OP_sapling_transaction_deprecated) {
    if (skip_pat(state, t)) tz_raise (MISMATCH);
    tz_continue;
  }
  state->micheline.pat_ofs+= 2;
  // read capture kind (6-bit positive integer)
  if (skip_pat(state, TZ_MICHELINE_TAG_INT))
    tz_raise (BAD_PATTERN);
  uint8_t kind;
  if (read_pat(state, &kind))
    tz_raise (BAD_PATTERN);
  if (kind & 0xC0)
    tz_raise (BAD_PATTERN);
  if (kind == TZ_CAP_OR) {
    if (t < TZ_MICHELINE_TAG_PRIM_0_NOANNOTS || t > TZ_MICHELINE_TAG_PRIM_N) tz_raise (MISMATCH);
    // bypass STEP_TAG parser
    state->micheline.frame->step = TZ_MICHELINE_STEP_BRANCHING;
    state->micheline.frame->step_branching.pat_ofs_after = 0;
    state->micheline.frame->step_branching.tag = t;
    tz_break;
  } else if (kind == TZ_CAP_LIST) {
    if (t != TZ_MICHELINE_TAG_SEQ) tz_raise (MISMATCH);
    // bypass STEP_TAG parser
    state->micheline.frame->step = TZ_MICHELINE_STEP_LISTING;
    state->micheline.frame->step_listing.saved_pat_ofs = state->micheline.pat_ofs;
    state->micheline.frame->step_listing.seq = -1;
    if (push_frame (state, TZ_MICHELINE_STEP_SIZE)) tz_reraise;
    state->micheline.frame->step_size.size = 0;
    state->micheline.frame->stop = state->ofs+4;
    tz_break;
  } else {
    // read capture name
    if (skip_pat(state, TZ_MICHELINE_TAG_STRING))
      tz_raise (BAD_PATTERN);
    uint16_t pat_name_size;
    tz_must (size_pat(state, &pat_name_size));
    if (pat_name_size >= TZ_CAPTURE_BUFFER_SIZE)
      tz_raise (BAD_PATTERN);
    int i;
    for(i=0;i<pat_name_size;i++) {
      if (read_pat(state, (uint8_t*) &state->field_name[i]))
        tz_raise (BAD_PATTERN);
    }
    state->field_name[i] = 0;
    // initialize parser state for capture
    state->micheline.frame->step = TZ_MICHELINE_STEP_CAPTURING;
    state->micheline.capturing = true;
    state->micheline.frame->step_capturing = kind;
    tz_must (push_frame (state, TZ_MICHELINE_STEP_TAG));
    switch(kind) {
    case TZ_CAP_STREAM_ANY:
      break;
    case TZ_CAP_STREAM_BYTES: {
      if (t != TZ_MICHELINE_TAG_BYTES) tz_raise (MISMATCH);
      break;
    }
    case TZ_CAP_STREAM_INT: {
      if (t != TZ_MICHELINE_TAG_INT) tz_raise (MISMATCH);
      break;
    }
    case TZ_CAP_STREAM_STRING: {
      if (t != TZ_MICHELINE_TAG_STRING) tz_raise (MISMATCH);
      break;
    }
    case TZ_CAP_ADDRESS: {
      if (t != TZ_MICHELINE_TAG_BYTES) tz_raise (MISMATCH);
      // bypass STEP_TAG parser
      state->micheline.frame->step = TZ_MICHELINE_STEP_CAPTURE_BYTES;
      state->micheline.frame->step_capture.ofs = 0;
      tz_must (push_frame (state, TZ_MICHELINE_STEP_SIZE));
      state->micheline.frame->step_size.size = 0;
      state->micheline.frame->stop = state->ofs+4;
      tz_break;
    }
    default:
      tz_raise (BAD_PATTERN);
    }
  }
  tz_continue;
}

static tz_parser_result stop_capture_bytes(tz_parser_state *state) {
  switch (state->micheline.frame[-1].step_capturing) {
  case TZ_CAP_ADDRESS:
    if (tz_format_address(state->buffers.capture, state->micheline.frame->step_capture.ofs, (char*) state->buffers.capture)) tz_raise (MISMATCH);
    break;
  default:
    tz_raise (INVALID_STATE);
  }
  state->micheline.frame->step = TZ_MICHELINE_STEP_PRINT_CAPTURE;
  state->micheline.frame->step_capture.ofs = 0;
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

static tz_parser_result parser_put_if_not_capturing(tz_parser_state *state, tz_parser_regs *regs, char c) {
#ifdef TEZOS_DEBUG
  PRINTF("[DEBUG] put%s(char: '%c',int: %d)\n", state->micheline.capturing ? "*" : "", c, (int) c);
#endif
  if (!state->micheline.capturing) return 0;
  return (tz_parser_put (state, regs, c));
}

static tz_parser_result tag_selection (tz_parser_state *state, uint8_t t) {
  switch (t) {
  case TZ_MICHELINE_TAG_INT: {
    if (skip_pat(state, t)) tz_raise (MISMATCH);
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

#ifdef TEZOS_DEBUG
  PRINTF("[DEBUG] micheline(frame: %d, offset:%d/%d, step: %s, errno: %s)\n",
         (int) (state->micheline.frame - state->micheline.stack),
         (int) state->ofs,
         (int) state->micheline.frame->stop,
         (const char*) PIC(tz_micheline_parser_step_name[state->micheline.frame->step]),
         tz_parser_result_name(state->errno));
#endif

  switch (state->micheline.frame->step) {
  case TZ_MICHELINE_STEP_INT: {
    uint8_t b;
    tz_must (tz_parser_read(state, regs,&b));
    if (skip_pat(state, b)) tz_raise (MISMATCH);
    tz_must (tz_parse_int_step (&state->buffers.num, &state->micheline.frame->step_int, b, state->micheline.capturing));
    if (state->micheline.frame->step_int.stop) {
      if (state->micheline.capturing) {
        state->micheline.frame->step = TZ_MICHELINE_STEP_PRINT_INT;
        state->micheline.frame->step_int.size = 0;
      } else {
        tz_must (pop_frame (state));
      }
    }
    break;
  }
  case TZ_MICHELINE_STEP_PRINT_INT: {
    if (state->micheline.frame->step_int.sign) {
      tz_must (parser_put_if_not_capturing(state, regs, '-'));
      state->micheline.frame->step_int.sign = 0;
    } else if (state->buffers.num.decimal[state->micheline.frame->step_int.size]) {
      tz_must (parser_put_if_not_capturing(state, regs, state->buffers.num.decimal[state->micheline.frame->step_int.size]));
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
        tz_must (parser_put_if_not_capturing(state, regs, '{'));
        state->micheline.frame->step_seq.first = false;
      } else {
        tz_must (parser_put_if_not_capturing(state, regs, '}'));
        tz_must (end_sized (state));
        tz_must (pop_frame (state));
      }
    } else {
      if (state->micheline.frame->step_seq.first) {
        tz_must (parser_put_if_not_capturing(state, regs, '{'));
        state->micheline.frame->step_seq.first = false;
      } else {
        tz_must (parser_put_if_not_capturing(state, regs, ';'));
      }
      tz_must (continue_sized (state));
      tz_must (push_frame (state, TZ_MICHELINE_STEP_TAG));
    }
    break;
  }
  case TZ_MICHELINE_STEP_LISTING: {
    if (state->micheline.frame->stop == state->ofs) {
      tz_must (pop_frame (state));
    } else {
      state->micheline.pat_ofs = state->micheline.frame->step_listing.saved_pat_ofs;
      state->micheline.frame->step_listing.seq++;
      tz_must (push_frame (state, TZ_MICHELINE_STEP_TAG));
    }
    break;
  }
  case TZ_MICHELINE_STEP_BRANCHING: {
    if (state->micheline.frame->step_branching.pat_ofs_after == 0) {
      uint8_t t = state->micheline.frame->step_branching.tag;
      uint8_t b;
      tz_must (tz_parser_peek(state, regs,&b));
      if ((unsigned) state->micheline.pat_ofs >= state->micheline.pat_len)
        tz_raise (BAD_PATTERN);
      if (state->micheline.pat[state->micheline.pat_ofs] != TZ_MICHELINE_TAG_SEQ)
        tz_raise (BAD_PATTERN);
      state->micheline.pat_ofs++;
      uint16_t stop;
      tz_must (size_pat(state, &stop));
      stop += state->micheline.pat_ofs;
      state->micheline.frame->step_branching.pat_ofs_after = stop;
      while(state->micheline.pat_ofs < stop) {
        if (state->micheline.pat[state->micheline.pat_ofs] != TZ_MICHELINE_TAG_SEQ)
          tz_raise (BAD_PATTERN);
        state->micheline.pat_ofs++;
        uint16_t sub;
        tz_must (size_pat(state, &sub));
        if (sub < 2
            || state->micheline.pat[state->micheline.pat_ofs] < TZ_MICHELINE_TAG_PRIM_0_NOANNOTS
            || state->micheline.pat[state->micheline.pat_ofs] > TZ_MICHELINE_TAG_PRIM_N) {
          tz_raise (BAD_PATTERN);
        }
        if (state->micheline.pat[state->micheline.pat_ofs+1] == b) {
          if (skip_pat(state, t)) tz_raise (MISMATCH);
          tz_must (push_frame (state, TZ_MICHELINE_STEP_TAG));
          tz_must (tag_selection(state, t));
          tz_continue;
        } else {
          state->micheline.pat_ofs += sub;
        }
      }
      tz_raise (MISMATCH);
    } else {
      state->micheline.pat_ofs = state->micheline.frame->step_branching.pat_ofs_after;
      tz_must (pop_frame (state));
    }
    break;
  }
  case TZ_MICHELINE_STEP_CAPTURING: {
    state->micheline.capturing = false;
    tz_must (pop_frame (state));
    tz_stop (IM_FULL);
  }
  case TZ_MICHELINE_STEP_PRINT_CAPTURE: {
    if (state->buffers.capture[state->micheline.frame->step_capture.ofs]) {
      tz_must (parser_put_if_not_capturing(state, regs, state->buffers.capture[state->micheline.frame->step_capture.ofs]));
      state->micheline.frame->step_capture.ofs++;
    } else {
      tz_must (pop_frame (state));
    }
    break;
  }
  case TZ_MICHELINE_STEP_CAPTURE_BYTES: {
    if (state->micheline.frame->stop == state->ofs) {
      tz_must (end_sized (state));
      tz_must (stop_capture_bytes (state));
    } else {
      if (state->micheline.frame->step_capture.ofs >= TZ_CAPTURE_BUFFER_SIZE) tz_raise (TOO_LARGE);
      tz_must (tz_parser_read(state,regs,&state->buffers.capture[state->micheline.frame->step_capture.ofs]));
      state->micheline.frame->step_capture.ofs++;
    }
    break;
  }
  case TZ_MICHELINE_STEP_BYTES: {
    if (state->micheline.frame->step_bytes.has_rem_half) {
      tz_must (parser_put_if_not_capturing(state, regs, state->micheline.frame->step_bytes.rem_half));
      state->micheline.frame->step_bytes.has_rem_half = 0;
    } else if (state->micheline.frame->step_bytes.first) {
      tz_must (parser_put_if_not_capturing(state, regs, '0'));
      state->micheline.frame->step_bytes.has_rem_half = true;
      state->micheline.frame->step_bytes.rem_half = 'x';
      state->micheline.frame->step_bytes.first = false;
    } else if (state->micheline.frame->stop == state->ofs) {
      tz_must (end_sized (state));
      tz_must (pop_frame (state));
    } else {
      uint8_t b;
      char half;
      tz_must (tz_parser_peek(state,regs,&b));
      half = hex_c[(b & 0xF0) >> 4];
      tz_must (parser_put_if_not_capturing(state, regs, half));
      state->micheline.frame->step_bytes.has_rem_half = true;
      state->micheline.frame->step_bytes.rem_half = hex_c[b & 0x0F];
      if (skip_pat(state, b)) tz_raise (MISMATCH);
      tz_parser_skip(state, regs);
    }
    break;
  }
  case TZ_MICHELINE_STEP_STRING: {
    if (state->micheline.frame->step_string.first) {
      tz_must (parser_put_if_not_capturing(state, regs, '\"'));
      state->micheline.frame->step_string.first = false;
    } else if (state->micheline.frame->stop == state->ofs) {
      tz_must (parser_put_if_not_capturing(state, regs, '\"'));
      tz_must (end_sized (state));
      tz_must (pop_frame (state));
    } else {
      uint8_t b;
      tz_must (tz_parser_peek(state,regs,&b));
      if (b >= 0x20 && b < 0x80 && b != '\"' && b != '\\') {
        tz_must (parser_put_if_not_capturing(state, regs, b));
        if (skip_pat(state, b)) tz_raise (MISMATCH);
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
      tz_must (end_sized (state));
      tz_must (pop_frame (state));
    } else {
      if (state->micheline.frame->step_annot.first) {
        tz_must (parser_put_if_not_capturing(state, regs, ' '));
        state->micheline.frame->step_annot.first = false;
      }
      uint8_t b;
      tz_must (tz_parser_peek(state,regs,&b));
      tz_must (parser_put_if_not_capturing(state, regs, b));
      if (skip_pat(state, b)) tz_raise (MISMATCH);
      tz_parser_skip(state, regs);
    }
    break;
  }
  case TZ_MICHELINE_STEP_PRIM_OP: {
    uint8_t op;
    tz_must (tz_parser_read(state, regs,&op));
    if (skip_pat(state, op)) tz_raise (MISMATCH);
    if (tz_michelson_op_name(op) == NULL) tz_raise (INVALID_OP);
    state->micheline.frame->step = TZ_MICHELINE_STEP_PRIM_NAME;
    state->micheline.frame->step_prim.op = op;
    break;
  }
  case TZ_MICHELINE_STEP_PRIM_NAME: {
    if (state->micheline.frame->step_prim.wrap && state->micheline.frame->step_prim.first) {
      tz_must (parser_put_if_not_capturing(state, regs, '('));
      state->micheline.frame->step_prim.first = false;
    }
    if (tz_michelson_op_name(state->micheline.frame->step_prim.op)[state->micheline.frame->step_prim.ofs]) {
      tz_must (parser_put_if_not_capturing(state, regs, tz_michelson_op_name(state->micheline.frame->step_prim.op)[state->micheline.frame->step_prim.ofs]));
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
      if (!state->micheline.capturing)
        if (state->micheline.frame->step_prim.nargs == 3 && state->micheline.frame->pat_stop != state->micheline.pat_ofs) tz_raise (MISMATCH);
      if (state->micheline.frame->step_prim.annot) {
        state->micheline.frame->step_prim.annot = false;
        tz_must (push_frame (state, TZ_MICHELINE_STEP_ANNOT));
        state->micheline.frame->step_annot.first = true;
        tz_must (begin_sized (state));
      } else {
        if (state->micheline.frame->step_prim.wrap)
          tz_must (parser_put_if_not_capturing(state, regs, ')'));
        tz_must (pop_frame (state));
      }
    } else if (!state->micheline.frame->step_prim.spc) {
      tz_must (parser_put_if_not_capturing(state, regs, ' '));
      state->micheline.frame->step_prim.spc = true;
    } else {
      if (state->micheline.frame->step_prim.nargs < 3) state->micheline.frame->step_prim.nargs--;
      state->micheline.frame->step_prim.spc = false;
      tz_must (continue_sized (state));
      tz_must (push_frame (state, TZ_MICHELINE_STEP_TAG));
    }
    break;
  }
  case TZ_MICHELINE_STEP_TAG: {
    uint8_t t;
    tz_must (tz_parser_read(state,regs,&t));
    tz_must (may_start_capture(state, t));
    tz_must (tag_selection(state, t));
    break;
    default: tz_raise (INVALID_STATE);
  }
  }
  tz_continue;
}
