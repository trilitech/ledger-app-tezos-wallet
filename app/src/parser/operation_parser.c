/* Tezos Embedded C parser for Ledger - Operation parser

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>
   Copyright 2023 TriliTech <contact@trili.tech>

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

/* Prototypes */

static tz_parser_result push_frame(tz_parser_state *,
                                   tz_operation_parser_step_kind);
static tz_parser_result pop_frame(tz_parser_state *);


#ifdef TEZOS_DEBUG
const char *const tz_operation_parser_step_name[] = {
    "MAGIC",
    "READ_BINARY",
    "BRANCH",
    "BATCH",
    "TAG",
    "SIZE",
    "OPERATION",
    "PRINT",
    "PARTIAL_PRINT",
    "READ_NUM",
    "READ_INT32",
    "READ_PK",
    "READ_BYTES",
    "READ_STRING",
    "READ_SMART_ENTRYPOINT",
    "READ_MICHELINE",
    "READ_SORU_MESSAGES",
    "READ_SORU_KIND",
    "READ_BALLOT",
    "READ_PROTOS"
};
#endif

const tz_operation_field_descriptor proposals_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Source",        TZ_OPERATION_FIELD_PKH,         true, false, false },
    { "Period",        TZ_OPERATION_FIELD_INT32,       true, false, false },
    { "Proposal",      TZ_OPERATION_FIELD_PROTOS,      true, false, false },
    { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor ballot_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Source",        TZ_OPERATION_FIELD_PKH,         true, false, false },
    { "Period",        TZ_OPERATION_FIELD_INT32,       true, false, false },
    { "Proposal",      TZ_OPERATION_FIELD_PROTO,       true, false, false },
    { "Ballot",        TZ_OPERATION_FIELD_BALLOT,      true, false, false },
    { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor failing_noop_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Message",       TZ_OPERATION_FIELD_BINARY,      true, false, false },
    { NULL, 0, 0, 0, 0 }
};

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
    { "Delegate",      TZ_OPERATION_FIELD_PKH,         false,false, true  },
    { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor reg_glb_cst_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
    { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
    { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
    { "Value",         TZ_OPERATION_FIELD_EXPR,        true, false, false },
    { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor set_deposit_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
    { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
    { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
    { "Staking limit", TZ_OPERATION_FIELD_AMOUNT,      false,false, true  },
    { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor inc_paid_stg_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
    { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
    { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
    { "Amount",        TZ_OPERATION_FIELD_INT,         true, false, false },
    { "Destination",   TZ_OPERATION_FIELD_DESTINATION, true, false, false },
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

const tz_operation_field_descriptor origination_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
    { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
    { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
    { "Balance",       TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
    { "Delegate",      TZ_OPERATION_FIELD_PKH,         false,false, true  },
    { "Code",          TZ_OPERATION_FIELD_EXPR,        true, false, false },
    { "Storage",       TZ_OPERATION_FIELD_EXPR,        true, false, false },
    { NULL, 0, 0, 0, 0 }
};


const tz_operation_field_descriptor transfer_tck_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
    { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
    { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
    { "Contents",      TZ_OPERATION_FIELD_EXPR,        true, false, false },
    { "Type",          TZ_OPERATION_FIELD_EXPR,        true, false, false },
    { "Ticketer",      TZ_OPERATION_FIELD_DESTINATION, true, false, false },
    { "Amount",        TZ_OPERATION_FIELD_NAT,         true, false, false },
    { "Destination",   TZ_OPERATION_FIELD_DESTINATION, true, false, false },
    { "Entrypoint",    TZ_OPERATION_FIELD_STRING,      true, false, false },
    { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor soru_add_msg_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Source",        TZ_OPERATION_FIELD_SOURCE,        true, true,  false },
    { "Fee",           TZ_OPERATION_FIELD_AMOUNT,        true, false, false },
    { "Counter",       TZ_OPERATION_FIELD_NAT,           true, true,  false },
    { "Gas",           TZ_OPERATION_FIELD_NAT,           true, true,  false },
    { "Storage limit", TZ_OPERATION_FIELD_NAT,           true, false, false },
    { "Message",       TZ_OPERATION_FIELD_SORU_MESSAGES, true, false, false },
    { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor soru_exe_msg_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
    { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
    { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
    { "Rollup",        TZ_OPERATION_FIELD_SR,          true, false, false },
    { "Commitment",    TZ_OPERATION_FIELD_SRC,         true, false, false },
    { "Output proof",  TZ_OPERATION_FIELD_BINARY,      true, false, false },
    { NULL, 0, 0, 0, 0 }
};

const tz_operation_field_descriptor soru_origin_fields[] = {
    // Name,           Kind,                        Req,  Skip,  None
    { "Source",        TZ_OPERATION_FIELD_SOURCE,      true, true,  false },
    { "Fee",           TZ_OPERATION_FIELD_AMOUNT,      true, false, false },
    { "Counter",       TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Gas",           TZ_OPERATION_FIELD_NAT,         true, true,  false },
    { "Storage limit", TZ_OPERATION_FIELD_NAT,         true, false, false },
    { "Kind",          TZ_OPERATION_FIELD_SORU_KIND,   true, false, false },
    { "Kernel",        TZ_OPERATION_FIELD_BINARY,      true, false, false },
    { "Proof",         TZ_OPERATION_FIELD_BINARY,      true, false, false },
    { "Parameters",    TZ_OPERATION_FIELD_EXPR,        true, false, false },
    { NULL, 0, 0, 0, 0 }
};

const tz_operation_descriptor tz_operation_descriptors[] = {
    { TZ_OPERATION_TAG_PROPOSALS,    "Proposals",                  proposals_fields    },
    { TZ_OPERATION_TAG_BALLOT,       "Ballot",                     ballot_fields       },
    { TZ_OPERATION_TAG_FAILING_NOOP, "Failing noop",               failing_noop_fields },
    { TZ_OPERATION_TAG_REVEAL,       "Reveal",                     reveal_fields       },
    { TZ_OPERATION_TAG_TRANSACTION,  "Transaction",                transaction_fields  },
    { TZ_OPERATION_TAG_ORIGINATION,  "Origination",                origination_fields  },
    { TZ_OPERATION_TAG_DELEGATION,   "Delegation",                 delegation_fields   },
    { TZ_OPERATION_TAG_REG_GLB_CST,  "Register global constant",   reg_glb_cst_fields  },
    { TZ_OPERATION_TAG_SET_DEPOSIT,  "Set deposit limit",          set_deposit_fields  },
    { TZ_OPERATION_TAG_INC_PAID_STG, "Increase paid storage",      inc_paid_stg_fields },
    { TZ_OPERATION_TAG_UPDATE_CK,    "Set consensus key",          update_ck_fields    },
    { TZ_OPERATION_TAG_TRANSFER_TCK, "Transfer ticket",            transfer_tck_fields },
    { TZ_OPERATION_TAG_SORU_ADD_MSG, "SR: send messages",          soru_add_msg_fields },
    { TZ_OPERATION_TAG_SORU_EXE_MSG, "SR: execute outbox message", soru_exe_msg_fields },
    { TZ_OPERATION_TAG_SORU_ORIGIN,  "SR: originate",              soru_origin_fields  },
    { 0, NULL, 0 }
};

static const char* expression_name = "Expression";
static const char* unset_message = "Field unset";

static tz_parser_result push_frame(tz_parser_state *state,
                                   tz_operation_parser_step_kind step) {
    tz_operation_state *op = &state->operation;

    if (op->frame >=
        &op->stack[TZ_OPERATION_STACK_DEPTH - 1])
        tz_raise(TOO_DEEP);
    op->frame++ ;
    op->frame->step = step;
    tz_continue;
}

static tz_parser_result pop_frame(tz_parser_state *state) {
    tz_operation_state *op = &state->operation;

    if (op->frame == op->stack) {
        op->frame = NULL;
      tz_stop(DONE);
    }
    op->frame--;
    tz_continue;
}

void tz_operation_parser_set_size(tz_parser_state *state, uint16_t size) {
    state->operation.stack[0].stop = size;
}

void tz_operation_parser_init(tz_parser_state *state, uint16_t size,
                              bool skip_magic) {
    tz_operation_state *op = &state->operation;

    tz_parser_init(state);
    state->operation.seen_reveal = 0;
    memset(&state->operation.source, 0, 22);
    memset(&state->operation.destination, 0, 22);
    op->batch_index = 0;
    op->frame = op->stack;
    op->stack[0].stop = size;
    if (!skip_magic) {
        op->stack[0].step = TZ_OPERATION_STEP_MAGIC;
    } else {
        strcpy(state->field_name, "Branch");
        op->stack[0].step = TZ_OPERATION_STEP_BRANCH;
        push_frame(state, TZ_OPERATION_STEP_READ_BYTES); // ignore result,
                                                         // assume success
        op->frame->step_read_bytes.kind = TZ_OPERATION_FIELD_BH;
        op->frame->step_read_bytes.skip = true;
        op->frame->step_read_bytes.ofs = 0;
        op->frame->step_read_bytes.len = 32;
    }
}

static tz_parser_result tz_print_string(tz_parser_state *state) {
    tz_operation_state *op = &state->operation;

    if (op->frame->step_read_string.skip) {
        tz_must(pop_frame(state));
        tz_continue;
    }
    op->frame->step = TZ_OPERATION_STEP_PRINT;
    op->frame->step_print.str = (char*) state->buffers.capture;
    tz_continue;
}

tz_parser_result tz_operation_parser_step(tz_parser_state *state) {
    tz_operation_state *op = &state->operation;
    tz_parser_regs *regs = &state->regs;
    uint8_t *capture = state->buffers.capture;

    // cannot restart after error
    if (TZ_IS_ERR(state->errno)) tz_reraise;

    // nothing else to do
    if (op->frame == NULL) tz_stop(DONE);

    PRINTF("[DEBUG] operation(frame: %d, offset:%d/%d, ilen: %d, olen: %d, "
           "step: %s, errno: %s)\n",
           (int) (op->frame - op->stack),
           (int) state->ofs,
           (int) op->stack[0].stop,
           (int) regs->ilen,
           (int) regs->oofs,
           (const char*)
             PIC(tz_operation_parser_step_name[op->frame->step]),
           tz_parser_result_name(state->errno));

    switch (op->frame->step) {
    case TZ_OPERATION_STEP_MAGIC: {
        uint8_t b;
        tz_must(tz_parser_read(state, &b));
        switch (b) {
        case 3: // manager/anonymous operation
            strcpy(state->field_name, "Branch");
            op->stack[0].step = TZ_OPERATION_STEP_BRANCH;
            push_frame(state, TZ_OPERATION_STEP_READ_BYTES); // ignore result,
                                                             //  assume success
            op->frame->step_read_bytes.kind = TZ_OPERATION_FIELD_BH;
            op->frame->step_read_bytes.skip = true;
            op->frame->step_read_bytes.ofs = 0;
            op->frame->step_read_bytes.len = 32;
            break;
        case 5: // micheline expression
            op->frame->step = TZ_OPERATION_STEP_READ_MICHELINE;
            op->frame->step_read_micheline.inited = 0;
            op->frame->step_read_micheline.skip = false;
            op->frame->step_read_micheline.name =
                (char*) PIC(expression_name);
            op->frame->stop = 0;
            break;
        default: tz_raise(INVALID_TAG);
        }
      break;
      }
    case TZ_OPERATION_STEP_SIZE: {
        uint8_t b;
        tz_must(tz_parser_read(state, &b));
        if (op->frame->step_size.size > 255)
            tz_raise(TOO_LARGE); // enforce 16-bit restriction
        op->frame->step_size.size = op->frame->step_size.size << 8 | b;
        op->frame->step_size.size_len--;
        if (op->frame->step_size.size_len <= 0) {
            op->frame[-1].stop =
            state->ofs + op->frame->step_size.size;
            tz_must(pop_frame(state));
        }
        break;
    }
    case TZ_OPERATION_STEP_TAG: {
        const tz_operation_descriptor* d;
        uint8_t t;
        tz_must(tz_parser_read(state, &t));
        for (d = tz_operation_descriptors; d->tag != TZ_OPERATION_TAG_END; d++) {
            if (d->tag == t) {
                op->frame->step = TZ_OPERATION_STEP_OPERATION;
                op->frame->step_operation.descriptor = d;
                op->frame->step_operation.field = 0;
                tz_must(push_frame(state, TZ_OPERATION_STEP_PRINT));
                snprintf(state->field_name, 30, "Operation (%d)",
                         op->batch_index);
                op->frame->step_print.str = d->name;
                tz_continue;
            }
        }
      tz_raise(INVALID_TAG);
    }
    case TZ_OPERATION_STEP_READ_MICHELINE: {
        if (!op->frame->step_read_micheline.inited) {
            op->frame->step_read_micheline.inited = 1;
            strcpy(state->field_name,
                   op->frame->step_read_micheline.name);
            tz_micheline_parser_init(state);
        }
        tz_micheline_parser_step(state);
        if (state->errno == TZ_BLO_DONE) {
            if (op->frame->stop != 0 &&
                state->ofs != op->frame->stop)
                tz_raise(TOO_LARGE);
            tz_must(pop_frame(state));
            if (regs->oofs > 0)
                tz_stop(IM_FULL);
            else tz_continue;
        }
        tz_reraise;
    }
    case TZ_OPERATION_STEP_READ_NUM: {
        uint8_t b;
        tz_must(tz_parser_read(state, &b));
        tz_must(tz_parse_num_step(&state->buffers.num,
                                  &op->frame->step_read_num.state, b,
                                  op->frame->step_read_num.natural));
        if (op->frame->step_read_num.state.stop) {
            if (op->frame->step_read_num.skip) {
                tz_must(pop_frame(state));
                tz_continue;
            }
            char *str = state->buffers.num.decimal;
            op->frame->step = TZ_OPERATION_STEP_PRINT;
            switch (op->frame->step_read_num.kind) {
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
                    for (j=len;j>=0;j--) str[j+pad] = str[j];
                    for (j=0;j<pad;j++) str[j] = '0';
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
                    while (str[len-1] == '0') { len--; str[len] = 0; }
                }
                add_currency:
                str[len] = ' ';
                str[len+1] = 't';
                str[len+2] = 'z';
                len += 3;
                str[len] = 0;
                break;
            }
            default: tz_raise(INVALID_STATE);
            }
            op->frame->step_print.str = str;
        }
        break;
    }
    case TZ_OPERATION_STEP_READ_INT32: {
        uint32_t value = 0;
        uint8_t b;
        for (int ofs = 0; ofs < 4; ofs++) {
            tz_must(tz_parser_read(state, &b));
            value = value << 8 | b;
        }
        snprintf((char*) capture, 30, "%d", value);
        tz_must(tz_print_string(state));
        break;
    }
    case TZ_OPERATION_STEP_READ_BYTES: {
        if (op->frame->step_read_bytes.ofs <
            op->frame->step_read_bytes.len) {
            uint8_t *c;
            c = &capture[op->frame->step_read_bytes.ofs];
            tz_must(tz_parser_read(state, c));
            op->frame->step_read_bytes.ofs++;
        } else {
            if (op->frame->step_read_num.skip) {
                tz_must(pop_frame(state));
                tz_continue;
            }
            switch (op->frame->step_read_bytes.kind) {
            case TZ_OPERATION_FIELD_SOURCE:
                memcpy(op->source, capture, 22);
                __attribute__((fallthrough));
            case TZ_OPERATION_FIELD_PKH:
                if (tz_format_pkh(capture, 21, (char*) capture))
                    tz_raise(INVALID_TAG);
                break;
            case TZ_OPERATION_FIELD_PK:
                if (tz_format_pk(capture, op->frame->step_read_bytes.len,
                                 (char*) capture))
                    tz_raise(INVALID_TAG);
                break;
            case TZ_OPERATION_FIELD_SR:
                if (tz_format_base58check("sr1", capture, 20, (char*) capture))
                    tz_raise(INVALID_TAG);
                break;
            case TZ_OPERATION_FIELD_SRC:
                if (tz_format_base58check("src1", capture, 32, (char*) capture))
                    tz_raise(INVALID_TAG);
                break;
            case TZ_OPERATION_FIELD_PROTO:
                if (tz_format_base58check("proto", capture, 32, (char*) capture))
                    tz_raise(INVALID_TAG);
                break;
            case TZ_OPERATION_FIELD_DESTINATION:
                memcpy(op->destination, capture, 22);
                if (tz_format_address(capture, 22, (char*) capture))
                    tz_raise(INVALID_TAG);
                break;
            case TZ_OPERATION_FIELD_OPH:
                if (tz_format_oph(capture, 32, (char*) capture))
                    tz_raise(INVALID_TAG);
                break;
            case TZ_OPERATION_FIELD_BH:
                if (tz_format_bh(capture, 32, (char*) capture))
                    tz_raise(INVALID_TAG);
                break;
            default: tz_raise(INVALID_STATE);
            }
            op->frame->step = TZ_OPERATION_STEP_PRINT;
            op->frame->step_print.str = (char*) capture;
        }
        break;
    }
    case TZ_OPERATION_STEP_BRANCH: {
        op->frame->step = TZ_OPERATION_STEP_BATCH;
        tz_must(push_frame(state, TZ_OPERATION_STEP_TAG));
        break;
    }
    case TZ_OPERATION_STEP_BATCH: {
        op->batch_index++;
        if (state->ofs == op->frame->stop) {
            tz_must(pop_frame(state));
        } else if (state->ofs > op->frame->stop) {
            tz_raise(TOO_LARGE);
        } else {
            tz_must(push_frame(state, TZ_OPERATION_STEP_TAG));
        }
        break;
    }
    case TZ_OPERATION_STEP_READ_STRING: {
        if (state->ofs == op->frame->stop) {
            capture[op->frame->step_read_string.ofs] = 0;
            tz_must(tz_print_string(state));
        } else {
            uint8_t b;
            tz_must(tz_parser_read(state, &b));
            capture[op->frame->step_read_string.ofs] = b;
            op->frame->step_read_string.ofs++;
        }
        break;
    }
    case TZ_OPERATION_STEP_READ_BINARY: {
        if (state->ofs == op->frame->stop) {
            capture[op->frame->step_read_string.ofs] = 0;
            tz_must(tz_print_string(state));
        } else if (op->frame->step_read_string.ofs + 2 >= TZ_CAPTURE_BUFFER_SIZE) {
            capture[op->frame->step_read_string.ofs] = 0;
            op->frame->step_read_string.ofs = 0;
            if (!op->frame->step_read_string.skip) {
                tz_must(push_frame(state, TZ_OPERATION_STEP_PARTIAL_PRINT));
                op->frame->step_print.str = (char*) capture;
            }
        } else {
            uint8_t b;
            tz_must(tz_parser_read(state, &b));
            char* buf = (char*) capture + op->frame->step_read_string.ofs;
            snprintf(buf, 4, "%02x", b);
            op->frame->step_read_string.ofs += 2;
        }
        break;
    }
    case TZ_OPERATION_STEP_READ_SMART_ENTRYPOINT: {
        uint8_t b;
        tz_must(tz_parser_read(state, &b));
        switch (b) {
        case 0:
            strcpy((char*) capture, "default");
            tz_must(tz_print_string(state));
            break;
        case 1:
            strcpy((char*) capture, "root");
            tz_must(tz_print_string(state));
            break;
        case 2:
            strcpy((char*) capture, "do");
            tz_must(tz_print_string(state));
            break;
        case 3:
            strcpy((char*) capture, "set_delegate");
            tz_must(tz_print_string(state));
            break;
        case 4:
            strcpy((char*) capture, "remove_delegate");
            tz_must(tz_print_string(state));
            break;
        case 5:
            strcpy((char*) capture, "deposit");
            tz_must(tz_print_string(state));
            break;
        case 0xFF:
            op->frame->step = TZ_OPERATION_STEP_READ_STRING;
            op->frame->step_read_string.ofs = 0;
            tz_must(push_frame(state, TZ_OPERATION_STEP_SIZE));
            op->frame->step_size.size = 0;
            op->frame->step_size.size_len = 1;
            break;
        default: tz_raise(INVALID_TAG);
        }
        break;
    }
    case TZ_OPERATION_STEP_OPERATION: {
        const tz_operation_descriptor *d =
          op->frame->step_operation.descriptor;
        const tz_operation_field_descriptor *field =
          PIC(&d->fields[op->frame->step_operation.field]);
        const char *name = PIC(field->name);

        // Remaining content from previous section - display this first.
        if (regs->oofs > 0)
            tz_stop(IM_FULL);

        if (name == NULL) {
            tz_must(pop_frame(state));
        } else {
            uint8_t present = 1;
            if (!field->required)
                tz_must(tz_parser_read(state, &present));
            if (!field->skip)
                strcpy(state->field_name, name);
            op->frame->step_operation.field++;
            if (!present) {
                if (field->display_none) {
                    if (field->skip) tz_raise(INVALID_STATE);
                    tz_must(push_frame(state, TZ_OPERATION_STEP_PRINT));
                    op->frame->step_print.str = (char*) unset_message;
                }
                tz_continue;
            }
            switch (field->kind) {
            case TZ_OPERATION_FIELD_BINARY: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_BINARY));
                op->frame->step_read_string.ofs = 0;
                op->frame->step_read_string.skip = field->skip;
                tz_must(push_frame(state, TZ_OPERATION_STEP_SIZE));
                op->frame->step_size.size = 0;
                op->frame->step_size.size_len = 4;
                break;
            }
            case TZ_OPERATION_FIELD_SOURCE:
            case TZ_OPERATION_FIELD_PKH: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_BYTES));
                op->frame->step_read_bytes.kind = field->kind;
                op->frame->step_read_bytes.skip = field->skip;
                op->frame->step_read_bytes.ofs = 0;
                op->frame->step_read_bytes.len = 21;
                break;
            }
            case TZ_OPERATION_FIELD_PK: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_PK));
                op->frame->step_read_bytes.skip = field->skip;
                break;
            }
            case TZ_OPERATION_FIELD_SR: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_BYTES));
                op->frame->step_read_bytes.kind = field->kind;
                op->frame->step_read_bytes.skip = field->skip;
                op->frame->step_read_bytes.ofs = 0;
                op->frame->step_read_bytes.len = 20;
                break;
            }
            case TZ_OPERATION_FIELD_SRC: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_BYTES));
                op->frame->step_read_bytes.kind = field->kind;
                op->frame->step_read_bytes.skip = field->skip;
                op->frame->step_read_bytes.ofs = 0;
                op->frame->step_read_bytes.len = 32;
                break;
            }
            case TZ_OPERATION_FIELD_PROTO: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_BYTES));
                op->frame->step_read_bytes.kind = field->kind;
                op->frame->step_read_bytes.skip = field->skip;
                op->frame->step_read_bytes.ofs = 0;
                op->frame->step_read_bytes.len = 32;
                break;
            }
            case TZ_OPERATION_FIELD_PROTOS: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_PROTOS));
                op->frame->step_read_list.name = name;
                op->frame->step_read_list.index = 0;
                op->frame->step_read_list.skip = field->skip;
                tz_must (push_frame(state, TZ_OPERATION_STEP_SIZE));
                op->frame->step_size.size = 0;
                op->frame->step_size.size_len = 4;
                break;
            }
            case TZ_OPERATION_FIELD_DESTINATION: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_BYTES));
                op->frame->step_read_bytes.kind = field->kind;
                op->frame->step_read_bytes.skip = field->skip;
                op->frame->step_read_bytes.ofs = 0;
                op->frame->step_read_bytes.len = 22;
                break;
            }
            case TZ_OPERATION_FIELD_NAT:
            case TZ_OPERATION_FIELD_AMOUNT: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_NUM));
                tz_parse_num_state_init(&state->buffers.num,
                                        &op->frame->step_read_num.state);
                op->frame->step_read_num.kind = field->kind;
                op->frame->step_read_num.skip = field->skip;
                op->frame->step_read_num.natural = 1;
                break;
            }
            case TZ_OPERATION_FIELD_INT: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_NUM));
                tz_parse_num_state_init(&state->buffers.num,
                                        &op->frame->step_read_num.state);
                op->frame->step_read_num.kind = field->kind;
                op->frame->step_read_num.skip = field->skip;
                op->frame->step_read_num.natural = 0;
                break;
            }
            case TZ_OPERATION_FIELD_INT32: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_INT32));
                op->frame->step_read_string.skip = field->skip;
                break;
            }
            case TZ_OPERATION_FIELD_PARAMETER: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_MICHELINE));
                op->frame->step_read_micheline.inited = 0;
                op->frame->step_read_micheline.skip = field->skip;
                op->frame->step_read_micheline.name = name;
                tz_must(push_frame(state, TZ_OPERATION_STEP_SIZE));
                op->frame->step_size.size = 0;
                op->frame->step_size.size_len = 4;
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_SMART_ENTRYPOINT));
                strcpy(state->field_name, "Entrypoint");
                op->frame->step_read_string.ofs = 0;
                op->frame->step_read_string.skip = field->skip;
                break;
            }
            case TZ_OPERATION_FIELD_EXPR: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_MICHELINE));
                op->frame->step_read_micheline.inited = 0;
                op->frame->step_read_micheline.skip = field->skip;
                op->frame->step_read_micheline.name = name;
                tz_must(push_frame(state, TZ_OPERATION_STEP_SIZE));
                op->frame->step_size.size = 0;
                op->frame->step_size.size_len = 4;
                break;
            }
            case TZ_OPERATION_FIELD_STRING: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_STRING));
                op->frame->step_read_string.ofs = 0;
                op->frame->step_read_string.skip = field->skip;
                tz_must(push_frame(state, TZ_OPERATION_STEP_SIZE));
                op->frame->step_size.size = 0;
                op->frame->step_size.size_len = 4;
                break;
            }
            case TZ_OPERATION_FIELD_SORU_MESSAGES: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_SORU_MESSAGES));
                op->frame->step_read_list.name = name;
                op->frame->step_read_list.index = 0;
                op->frame->step_read_list.skip = field->skip;
                tz_must(push_frame(state, TZ_OPERATION_STEP_SIZE));
                op->frame->step_size.size = 0;
                op->frame->step_size.size_len = 4;
                break;
            }
            case TZ_OPERATION_FIELD_SORU_KIND: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_SORU_KIND));
                op->frame->step_read_string.skip = field->skip;
                break;
            }
            case TZ_OPERATION_FIELD_BALLOT: {
                tz_must(push_frame(state, TZ_OPERATION_STEP_READ_BALLOT));
                op->frame->step_read_string.skip = field->skip;
                break;
            }
            default: tz_raise(INVALID_STATE);
            }
        }
        break;
    }
    case TZ_OPERATION_STEP_READ_PK: {
        uint8_t b;
        tz_must(tz_parser_peek(state, &b));
        op->frame->step_read_bytes.kind = TZ_OPERATION_FIELD_PK;
        op->frame->step_read_bytes.ofs = 0;
        switch (b){
        case 0: // edpk
            op->frame->step_read_bytes.len = 33;
            break;
        case 1: // sppk
            op->frame->step_read_bytes.len = 34;
            break;
        case 2: // p2pk
            op->frame->step_read_bytes.len = 34;
            break;
        case 3: // BLpk
            op->frame->step_read_bytes.len = 49;
            break;
        default:
            tz_raise(INVALID_TAG);
        }
        op->frame->step = TZ_OPERATION_STEP_READ_BYTES;
        break;
    }
    case TZ_OPERATION_STEP_READ_SORU_MESSAGES: {
        uint8_t skip = op->frame->step_read_list.skip;
        const char* name = op->frame->step_read_list.name;
        uint16_t index = op->frame->step_read_list.index;

        // Remaining content from previous message - display this first.
        if (regs->oofs > 0)
            tz_stop(IM_FULL);

        if (op->frame->stop == state->ofs) {
            tz_must(pop_frame(state));
        } else {
            op->frame->step_read_list.index++;
            tz_must(push_frame(state, TZ_OPERATION_STEP_READ_BINARY));
            snprintf(state->field_name, TZ_FIELD_NAME_SIZE, "%s (%d)", name, index);
            op->frame->step_read_string.ofs = 0;
            op->frame->step_read_string.skip = skip;
            tz_must(push_frame(state, TZ_OPERATION_STEP_SIZE));
            op->frame->step_size.size = 0;
            op->frame->step_size.size_len = 4;
        }
        break;
    }
    case TZ_OPERATION_STEP_READ_SORU_KIND: {
        uint8_t b;
        tz_must(tz_parser_read(state, &b));
        switch(b) {
        case 0:
            strcpy((char*) capture, "arith");
            break;
        case 1:
            strcpy((char*) capture, "wasm_2_0_0");
            break;
        default: tz_raise(INVALID_TAG);
        }
        tz_must(tz_print_string(state));
        break;
    }
    case TZ_OPERATION_STEP_READ_BALLOT: {
        uint8_t b;
        tz_must(tz_parser_read(state, &b));
        switch(b) {
        case 0:
            strcpy((char*) capture, "yay");
            break;
        case 1:
            strcpy((char*) capture, "nay");
            break;
        case 2:
            strcpy((char*) capture, "pass");
            break;
        default: tz_raise(INVALID_TAG);
        }
        tz_must(tz_print_string(state));
        break;
    }
    case TZ_OPERATION_STEP_READ_PROTOS: {
        uint8_t skip = op->frame->step_read_list.skip;
        const char* name = op->frame->step_read_list.name;
        uint16_t index = op->frame->step_read_list.index;

        // Remaining content from previous proto - display this first.
        if (regs->oofs > 0)
            tz_stop(IM_FULL);

        if (op->frame->stop == state->ofs) {
            tz_must(pop_frame(state));
        } else {
            op->frame->step_read_list.index++;
            tz_must(push_frame(state, TZ_OPERATION_STEP_READ_BYTES));
            snprintf(state->field_name, 30, "%s (%d)", name, index);
            op->frame->step_read_bytes.kind = TZ_OPERATION_FIELD_PROTO;
            op->frame->step_read_bytes.skip = skip;
            op->frame->step_read_bytes.ofs = 0;
            op->frame->step_read_bytes.len = 32;
        }
        break;
    }
    case TZ_OPERATION_STEP_PRINT: {
        const char* str = PIC(op->frame->step_print.str);
        if (*str) {
            tz_must(tz_parser_put(state, *str));
            op->frame->step_print.str++;
        } else {
            tz_must(pop_frame(state));
            tz_stop(IM_FULL);
        }
        break;
    }
    case TZ_OPERATION_STEP_PARTIAL_PRINT: {
        const char* str = PIC(op->frame->step_print.str);
        if (*str) {
            tz_must(tz_parser_put(state, *str));
            op->frame->step_print.str++;
        } else {
            tz_must(pop_frame(state));
        }
        break;
    }
    default: tz_raise(INVALID_STATE);
    }
    tz_continue;
}
