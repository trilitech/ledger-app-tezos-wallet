/* Tezos Embedded C parser for Ledger - Operation parser

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

#include "operation_parser.h"
#include "micheline_parser.h"
#include "num_parser.h"
#include "pattern_registry.h"

#ifdef TEZOS_DEBUG
const char *const tz_operation_parser_step_name[] = {
  "MAGIC",
  "BRANCH",
  "BATCH",
  "TAG",
  "SIZE",
  "OPERATION",
  "PRINT",
  "READ_NUM",
  "READ_PK",
  "READ_BYTES",
  "READ_ENTRYPOINT",
  "READ_MICHELINE"
};
#endif

const tz_operation_field_descriptor transaction_fields[] = {
  // Name,           Kind,                        Req,  Skip,  None
  { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
  { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
  { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
  { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
  { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
  { "Amount",        TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
  { "Destination",   TZ_OPERATION_FIELD_DESTINATION, true, false, false },
  { "Parameter",     TZ_OPERATION_FIELD_PARAMETER,   false,false, false },
  { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor reveal_fields[] = {
  // Name,           Kind,                        Req,  Skip,  None
  { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
  { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
  { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
  { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
  { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
  { "Public key",    TZ_OPERATION_FIELD_PK,          true, false, false },
  { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor delegation_fields[] = {
  // Name,           Kind,                        Req,  Skip,  None
  { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
  { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
  { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
  { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
  { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
  { "Delegate",      TZ_OPERATION_FIELD_PKH,         false,false, true },
  { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor set_deposit_fields[] = {
  // Name,           Kind,                        Req,  Skip,  None
  { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
  { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
  { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
  { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
  { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
  { "Staking limit", TZ_OPERATION_FIELD_AMOUNT,      false,false, true },
  { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor update_ck_fields[] = {
  // Name,           Kind,                        Req,  Skip,  None
  { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
  { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
  { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
  { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
  { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
  { "Public key",    TZ_OPERATION_FIELD_PK,          true, false, false },
  { NULL, 0, 0, 0, 0 }
};


const tz_operation_descriptor tz_operation_descriptors[] = {
  { TZ_OPERATION_TAG_TRANSACTION, "Transaction",       transaction_fields  },
  { TZ_OPERATION_TAG_REVEAL,      "Reveal",            reveal_fields       },
  { TZ_OPERATION_TAG_DELEGATION,  "Delegation",        delegation_fields   },
  { TZ_OPERATION_TAG_SET_DEPOSIT, "Set deposit limit", set_deposit_fields  },
  { TZ_OPERATION_TAG_UPDATE_CK,   "Set consensus key", update_ck_fields    },
  { 0, NULL, 0 }
};

static const char* parameter_name = "Parameter";
static const char* expression_name = "Expression";
static const char* unset_message = "Field unset";

static tz_parser_result push_frame (tz_parser_state *state, tz_operation_parser_step_kind step) {
  if (state->operation.frame >= &state->operation.stack[TZ_OPERATION_STACK_DEPTH - 1])
    tz_raise (TOO_DEEP);
  state->operation.frame++ ;
  state->operation.frame->step = step;
  tz_continue;
}

static tz_parser_result pop_frame (tz_parser_state *state) {
  if (state->operation.frame == state->operation.stack) {
    state->operation.frame = NULL;
    tz_stop (DONE);
  }
  state->operation.frame--;
  tz_continue;
}

void tz_operation_parser_set_size(tz_parser_state *state, uint16_t size) {
  state->operation.stack[0].stop = size;
}

void tz_operation_parser_init(tz_parser_state *state, uint16_t size, bool skip_magic) {
  tz_parser_init(state);
  state->operation.seen_reveal = 0;
  memset(&state->operation.source, 0, 22);
  memset(&state->operation.destination, 0, 22);
  state->operation.batch_index = 0;
  state->operation.frame = state->operation.stack;
  state->operation.stack[0].stop = size;
  if (!skip_magic) {
    state->operation.stack[0].step = TZ_OPERATION_STEP_MAGIC;
  } else {
    strcpy(state->field_name, "Branch");
    state->operation.stack[0].step = TZ_OPERATION_STEP_BRANCH;
    push_frame(state, TZ_OPERATION_STEP_READ_BYTES); // ignore result, assume success
    state->operation.frame->step_read_bytes.kind = TZ_OPERATION_FIELD_BH;
    state->operation.frame->step_read_bytes.skip = true;
    state->operation.frame->step_read_bytes.ofs = 0;
    state->operation.frame->step_read_bytes.len = 32;
  }
}

tz_parser_result tz_operation_parser_step(tz_parser_state *state, tz_parser_regs *regs) {

  // cannot restart after error
  if (TZ_IS_ERR(state->errno)) tz_reraise;

  // nothing else to do
  if (state->operation.frame == NULL) tz_stop (DONE);

#ifdef TEZOS_DEBUG
  PRINTF("[DEBUG] operation(frame: %d, offset:%d/%d, ilen: %d, step: %s, errno: %s)\n",
         (int) (state->operation.frame - state->operation.stack),
         (int) state->ofs,
         (int) state->operation.stack[0].stop,
         (int) regs->ilen,
         (const char*) PIC(tz_operation_parser_step_name[state->operation.frame->step]),
         tz_parser_result_name(state->errno));
#endif

  switch (state->operation.frame->step) {
  case TZ_OPERATION_STEP_MAGIC: {
    uint8_t b;
    tz_must (tz_parser_read(state, regs,&b));
    switch (b) {
    case 3: // manager/anonymous operation
      strcpy(state->field_name, "Branch");
      state->operation.stack[0].step = TZ_OPERATION_STEP_BRANCH;
      push_frame(state, TZ_OPERATION_STEP_READ_BYTES); // ignore result, assume success
      state->operation.frame->step_read_bytes.kind = TZ_OPERATION_FIELD_BH;
      state->operation.frame->step_read_bytes.skip = true;
      state->operation.frame->step_read_bytes.ofs = 0;
      state->operation.frame->step_read_bytes.len = 32;
      break;
    case 5: // micheline expression
      state->operation.frame->step = TZ_OPERATION_STEP_READ_MICHELINE;
      state->operation.frame->step_read_micheline.inited = 0;
      state->operation.frame->step_read_micheline.skip = false;
      state->operation.frame->step_read_micheline.name = (char*) PIC(expression_name);
      state->operation.frame->step_read_micheline.pattern_entrypoint = NULL;
      state->operation.frame->stop = 0;
      break;
    default: tz_raise (INVALID_TAG);
    }
    break;
  }
  case TZ_OPERATION_STEP_SIZE: {
    uint8_t b;
    tz_must (tz_parser_read(state, regs,&b));
    if (state->operation.frame->step_size.size > 255) tz_raise (TOO_LARGE); // enforce 16-bit restriction
    state->operation.frame->step_size.size = state->operation.frame->step_size.size << 8 | b;
    if (state->operation.frame->stop == state->ofs) {
      state->operation.frame[-1].stop = state->ofs + state->operation.frame->step_size.size;
      tz_must (pop_frame (state));
    }
    break;
  }
  case TZ_OPERATION_STEP_TAG: {
    uint8_t t;
    tz_must (tz_parser_read(state,regs,&t));
    for (const tz_operation_descriptor* d = tz_operation_descriptors ; d->tag != 0 ; d++) {
      if (d->tag == t) {
        state->operation.frame->step = TZ_OPERATION_STEP_OPERATION;
        state->operation.frame->step_operation.descriptor = d;
        state->operation.frame->step_operation.field = 0;
        tz_must (push_frame(state, TZ_OPERATION_STEP_PRINT));
        snprintf (state->field_name, 30, "Operation (%d)", state->operation.batch_index);
        state->operation.frame->step_print.str = d->name;
        tz_continue;
      }
    }
    tz_raise (INVALID_TAG);
    break;
  }
  case TZ_OPERATION_STEP_READ_MICHELINE: {
    if (!state->operation.frame->step_read_micheline.inited) {
      state->operation.frame->step_read_micheline.inited = 1;
      strcpy(state->field_name, state->operation.frame->step_read_micheline.name);
      const uint8_t* pattern = NULL;
      uint16_t length = 0;
      if (state->operation.frame->step_read_micheline.pattern_entrypoint) {
        find_pattern(state->operation.destination, state->operation.frame->step_read_micheline.pattern_entrypoint, &pattern, &length);
      }
      tz_micheline_parser_init(state, pattern, length);
    }
    tz_micheline_parser_step(state, regs);
    if (state->errno == TZ_BLO_DONE) {
      if (state->operation.frame->stop != 0 && state->ofs != state->operation.frame->stop)
        tz_raise (MISMATCH);
      tz_must (pop_frame(state));
      if (regs->oofs > 0)
        tz_stop (IM_FULL);
      else tz_continue;
    }
    tz_reraise;
    break;
  }
  case TZ_OPERATION_STEP_READ_NUM: {
    uint8_t b;
    tz_must (tz_parser_read(state, regs,&b));
    tz_must (tz_parse_num_step (&state->buffers.num, &state->operation.frame->step_read_num.state, b, state->operation.frame->step_read_num.natural, 1));
    if (state->operation.frame->step_read_num.state.stop) {
      if (state->operation.frame->step_read_num.skip) {
        tz_must (pop_frame (state));
        tz_continue;
      }
      char *str = state->buffers.num.decimal;
      state->operation.frame->step = TZ_OPERATION_STEP_PRINT;
      switch (state->operation.frame->step_read_num.kind) {
      case TZ_OPERATION_FIELD_INT:
      case TZ_OPERATION_FIELD_NAT: break;
      case TZ_OPERATION_FIELD_AMOUNT: {
        int len = 0;
        while (str[len])len++;
        if (len == 1 && str[0] == 0) 
          // just 0
          goto add_currency;
        if (len < 7) {
          // less than one tez, pad left up to the '0.'
          int j;
          int pad = 7 - len;
          for(j=len;j>=0;j--) str[j+pad] = str[j];
          for(j=0;j<pad;j++) str[j] = '0';
          len = 7;
        }
        int no_decimals = 1;
        for (int i=0;i<6;i++) no_decimals &= (str[len-1-i] == '0');
        if (no_decimals) {
          // integral value, don't include the decimal part (no '.'_
          str[len - 6] = 0;
          len -= 6;
        } else {
          // more than one tez, add the '.'
          for (int i=0;i<6;i++) str[len-i] = str[len-i-1];
          str[len - 6] = '.';
          len++;
          str[len] = 0;
          // drop trailing non significant zeroes
          while(str[len-1] == '0') { len--; str[len] = 0; }
        }
        add_currency:
        str[len] = ' ';
        str[len+1] = 't';
        str[len+2] = 'z';
        len += 3;
        str[len] = 0;
        break;
      }
      default: tz_raise (INVALID_STATE);
      }
      state->operation.frame->step_print.str = str;
    }
    break;
  }
  case TZ_OPERATION_STEP_READ_BYTES: {
    if (state->operation.frame->step_read_bytes.ofs < state->operation.frame->step_read_bytes.len) {
      uint8_t *c = &state->buffers.capture[state->operation.frame->step_read_bytes.ofs];
      tz_must (tz_parser_read(state,regs,c));
      state->operation.frame->step_read_bytes.ofs++;
    } else {
      if (state->operation.frame->step_read_num.skip) {
        tz_must (pop_frame (state));
        tz_continue;
      }
      switch (state->operation.frame->step_read_bytes.kind) {
      case TZ_OPERATION_FIELD_SOURCE:
        memcpy(state->operation.source, state->buffers.capture, 22);
        // fallthrough
      case TZ_OPERATION_FIELD_PKH:
        if (tz_format_pkh (state->buffers.capture, 21, (char*) state->buffers.capture)) tz_raise (INVALID_TAG);
        break;
      case TZ_OPERATION_FIELD_PK:
        if (tz_format_pk (state->buffers.capture, state->operation.frame->step_read_bytes.len, (char*) state->buffers.capture)) tz_raise (INVALID_TAG);
        break;
      case TZ_OPERATION_FIELD_DESTINATION:
        memcpy(state->operation.destination, state->buffers.capture, 22);
        const char* name;
        if (find_name(state->buffers.capture, &name)) {
          strcpy((char*) state->buffers.capture, name);
        } else {
          if (tz_format_address (state->buffers.capture, 22, (char*) state->buffers.capture)) tz_raise (INVALID_TAG);
        }
        break;
      case TZ_OPERATION_FIELD_OPH:
        if (tz_format_oph (state->buffers.capture, 32, (char*) state->buffers.capture)) tz_raise (INVALID_TAG);
        break;
      case TZ_OPERATION_FIELD_BH:
        if (tz_format_bh (state->buffers.capture, 32, (char*) state->buffers.capture)) tz_raise (INVALID_TAG);
        break;
      default: tz_raise (INVALID_STATE);
      }
      state->operation.frame->step = TZ_OPERATION_STEP_PRINT;
      state->operation.frame->step_print.str = (char*) state->buffers.capture;
    }
    break;
  }
  case TZ_OPERATION_STEP_BRANCH: {
    state->operation.frame->step = TZ_OPERATION_STEP_BATCH;
    tz_must (push_frame (state, TZ_OPERATION_STEP_TAG));
    break;
  }
  case TZ_OPERATION_STEP_BATCH: {
    state->operation.batch_index++;
    if (state->ofs == state->operation.frame->stop) {
      tz_must (pop_frame (state));
    } else if (state->ofs > state->operation.frame->stop) {
      tz_raise (TOO_LARGE);
    } else {
      tz_must (push_frame (state, TZ_OPERATION_STEP_TAG));
    }
    break;
  }
  case TZ_OPERATION_STEP_READ_ENTRYPOINT: {
    uint8_t b;
    tz_must (tz_parser_read(state,regs,&b));
    if (state->operation.frame->step_read_entrypoint.ofs == 0) {
      switch(b) {
      case 0: strcpy((char*) state->buffers.capture+2, "default"); goto print_entrypoint;
      case 1: strcpy((char*) state->buffers.capture+2, "root"); goto print_entrypoint;
      case 2: strcpy((char*) state->buffers.capture+2, "do"); goto print_entrypoint;
      case 3: strcpy((char*) state->buffers.capture+2, "set_delegate"); goto print_entrypoint;
      case 4: strcpy((char*) state->buffers.capture+2, "remove_delegate"); goto print_entrypoint;
      case 5: strcpy((char*) state->buffers.capture+2, "deposit"); goto print_entrypoint;
      case 0xFF:
        state->operation.frame->step_read_entrypoint.ofs++;
        break;
      default: tz_raise (INVALID_TAG);
      }
    } else {
      state->buffers.capture[state->operation.frame->step_read_entrypoint.ofs] = b;
      state->operation.frame->step_read_entrypoint.ofs++;
      if (state->operation.frame->step_read_entrypoint.ofs - 2 == (int) state->buffers.capture[1]) {
        state->buffers.capture[state->operation.frame->step_read_entrypoint.ofs] = 0;
      print_entrypoint: {
          uint8_t skip = state->operation.frame->step_read_entrypoint.skip;
          state->operation.frame->step = TZ_OPERATION_STEP_READ_MICHELINE;
          state->operation.frame->step_read_micheline.inited = 0;
          state->operation.frame->step_read_micheline.skip = skip;
          state->operation.frame->step_read_micheline.name = (char*) PIC(parameter_name);
          state->operation.frame->step_read_micheline.pattern_entrypoint = (char*) state->buffers.capture+2;
          tz_must (push_frame(state, TZ_OPERATION_STEP_SIZE));
          state->operation.frame->step_size.size = 0;
          state->operation.frame->stop = state->ofs+4;
          if (!skip) {
            tz_must (push_frame(state, TZ_OPERATION_STEP_PRINT));
            strcpy(state->field_name, "Entrypoint");
            state->operation.frame->step_print.str = (char*) state->buffers.capture+2;
          }
        }
      }
    }
    break;
  }
  case TZ_OPERATION_STEP_OPERATION: {
    const tz_operation_descriptor *d =
      state->operation.frame->step_operation.descriptor;
    const tz_operation_field_descriptor *field =
      PIC(&d->fields[state->operation.frame->step_operation.field]);
    const char *name = PIC(field->name);
    if (name == NULL) {
      tz_must (pop_frame (state));
    } else {
      uint8_t present = 1;
      if (!field->required)
        tz_must (tz_parser_read(state,regs,&present));
      if (!field->skip)
        strcpy(state->field_name, name);
      state->operation.frame->step_operation.field++;
      if (!present) {
        if (field->display_none) {
          if (field->skip) tz_raise (INVALID_STATE);
          tz_must (push_frame(state, TZ_OPERATION_STEP_PRINT));
          state->operation.frame->step_print.str = (char*) unset_message;
        }
        tz_continue;
      }
      switch (field->kind) {
      case TZ_OPERATION_FIELD_SOURCE:
      case TZ_OPERATION_FIELD_PKH: {
        tz_must (push_frame(state, TZ_OPERATION_STEP_READ_BYTES));
        state->operation.frame->step_read_bytes.kind = field->kind;
        state->operation.frame->step_read_bytes.skip = field->skip;
        state->operation.frame->step_read_bytes.ofs = 0;
        state->operation.frame->step_read_bytes.len = 21;
        break;
      }
      case TZ_OPERATION_FIELD_PK: {
        tz_must (push_frame(state, TZ_OPERATION_STEP_READ_PK));
        state->operation.frame->step_read_bytes.skip = field->skip;
        break;
      }
      case TZ_OPERATION_FIELD_DESTINATION: {
        tz_must (push_frame(state, TZ_OPERATION_STEP_READ_BYTES));
        state->operation.frame->step_read_bytes.kind = field->kind;
        state->operation.frame->step_read_bytes.skip = field->skip;
        state->operation.frame->step_read_bytes.ofs = 0;
        state->operation.frame->step_read_bytes.len = 22;
        break;
      }
      case TZ_OPERATION_FIELD_NAT:
      case TZ_OPERATION_FIELD_AMOUNT: {
        tz_must (push_frame(state, TZ_OPERATION_STEP_READ_NUM));
        tz_parse_num_state_init(&state->buffers.num, &state->operation.frame->step_read_num.state);
        state->operation.frame->step_read_num.kind = field->kind;
        state->operation.frame->step_read_num.skip = field->skip;
        state->operation.frame->step_read_num.natural = 1;
        break;
      }
      case TZ_OPERATION_FIELD_INT: {
        tz_must (push_frame(state, TZ_OPERATION_STEP_READ_NUM));
        tz_parse_num_state_init(&state->buffers.num, &state->operation.frame->step_read_num.state);
        state->operation.frame->step_read_num.kind = field->kind;
        state->operation.frame->step_read_num.skip = field->skip;
        state->operation.frame->step_read_num.natural = 0;
        break;
      }
      case TZ_OPERATION_FIELD_PARAMETER: {
        tz_must (push_frame(state, TZ_OPERATION_STEP_READ_ENTRYPOINT));
        state->operation.frame->step_read_entrypoint.ofs = 0;
        state->operation.frame->step_read_entrypoint.skip = field->skip;
        break;
      }
      default: tz_raise (INVALID_STATE);
      }
    }
    break;
  }
  case TZ_OPERATION_STEP_READ_PK: {
    uint8_t b;
    tz_must (tz_parser_peek(state,regs,&b));
    state->operation.frame->step_read_bytes.kind = TZ_OPERATION_FIELD_PK;
    state->operation.frame->step_read_bytes.ofs = 0;
    switch (b){
    case 0: // edpk
      state->operation.frame->step_read_bytes.len = 33;
      break;
    case 1: // sppk
      state->operation.frame->step_read_bytes.len = 65;
      break;
    case 2: // p2pk
      state->operation.frame->step_read_bytes.len = 65;
      break;
    case 3: // BLpk
      state->operation.frame->step_read_bytes.len = 49;
      break;
    default:
      tz_raise (INVALID_TAG);
    }
    state->operation.frame->step = TZ_OPERATION_STEP_READ_BYTES;
    break;
  }
  case TZ_OPERATION_STEP_PRINT: {
    const char* str = PIC(state->operation.frame->step_print.str);
    if (*str) {
      tz_must (tz_parser_put(state, regs, *str));
      state->operation.frame->step_print.str++;
    } else {
      tz_must (pop_frame (state));
      tz_stop (IM_FULL);
    }
    break;
  }
  default: tz_raise (INVALID_STATE);
  }
  tz_continue;
}
