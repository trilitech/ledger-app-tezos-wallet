/* Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>

#include "micheline_parser.h"
#include "operation_parser.h"

CAMLprim value micheline_cparse_capture_name(value mlstate) {
  CAMLparam1(mlstate);
  CAMLlocal1(r);
  tz_parser_state **state = Data_abstract_val(mlstate);
  r = caml_copy_string((*state)->field_name);
  CAMLreturn(r);
}

CAMLprim value micheline_cparse_init(value size) {
  CAMLparam1(size);
  CAMLlocal1(r);
  r = caml_alloc(sizeof(value), Abstract_tag);
  tz_parser_state *state = malloc(sizeof(tz_parser_state));
  *((tz_parser_state**) Data_abstract_val(r)) = state;
  tz_micheline_parser_init(state);
  size_t s = Long_val(size);
  if (s >= 0xFFFF) caml_failwith("micheline_cparse_init: size too large");
  tz_operation_parser_init(state, (uint16_t) s, 1);
  CAMLreturn(r);
}

CAMLprim value micheline_cparse_step(value mlstate, value input, value output) {
  CAMLparam3(mlstate, input, output);
  CAMLlocal3(ibuf, obuf, r);
  ibuf = Field(input, 0);
  int iofs = Int_val(Field(input, 1));
  int ilen = Int_val(Field(input, 2));
  obuf = Field(output, 0);
  int oofs = Int_val(Field(output, 1));
  int olen = Int_val(Field(output, 2));
  // Data_abstract_val() returns a value that could change.
  // It must be dereferenced before being used.
  // https://gitlab.com/nomadic-labs/tezos-ledger-app-revamp/-/merge_requests/58#note_1434368804
  tz_parser_state *state = *((tz_parser_state**) Data_abstract_val(mlstate));

  state->regs.ibuf = Bytes_val(ibuf);
  state->regs.iofs = iofs;
  state->regs.ilen = ilen;
  state->regs.obuf = (char*) Bytes_val(obuf);
  state->regs.oofs = oofs;
  state->regs.olen = olen;

  while (!TZ_IS_BLOCKED(tz_micheline_parser_step(state)));

  int read = ilen - state->regs.ilen;
  int written = olen - state->regs.olen;

  r = caml_alloc_tuple(3);
  Store_field(r, 0, Val_int(read));
  Store_field(r, 1, Val_int(written));

  switch (state->errno) {
  case TZ_BLO_DONE:
    Store_field(r, 2, Val_int(0));
    break;
  case TZ_BLO_FEED_ME:
    Store_field(r, 2, Val_int(1));
    break;
  case TZ_BLO_IM_FULL:
    Store_field(r, 2, Val_int(2));
    break;
  case TZ_ERR_INVALID_TAG:
    caml_failwith("micheline_cparse_step: invalid tag");
  case TZ_ERR_INVALID_OP:
    caml_failwith("micheline_cparse_step: invalid operation");
  case TZ_ERR_UNSUPPORTED:
    caml_failwith("micheline_cparse_step: unsupported data");
  case TZ_ERR_TOO_LARGE:
    caml_failwith("micheline_cparse_step: data size limitation exceeded");
  case TZ_ERR_TOO_DEEP:
    caml_failwith("micheline_cparse_step: expression too deep");
  case TZ_ERR_INVALID_STATE:
    caml_failwith("micheline_cparse_step: invalid state");
  default:
    char err[100];
    snprintf(err, 99, "micheline_cparse_step: unknown error code %d", state->errno);
    caml_failwith(err);
  }
  CAMLreturn(r);
}

CAMLprim value operation_cparse_step(value mlstate, value input, value output) {
  CAMLparam3(mlstate, input, output);
  CAMLlocal3(ibuf, obuf, r);
  ibuf = Field(input, 0);
  int iofs = Int_val(Field(input, 1));
  int ilen = Int_val(Field(input, 2));
  obuf = Field(output, 0);
  int oofs = Int_val(Field(output, 1));
  int olen = Int_val(Field(output, 2));
  // Data_abstract_val() returns a value that could change.
  // It must be dereferenced before being used.
  // https://gitlab.com/nomadic-labs/tezos-ledger-app-revamp/-/merge_requests/58#note_1434368804
  tz_parser_state *state = *((tz_parser_state**) Data_abstract_val(mlstate));

  state->regs.ibuf = Bytes_val(ibuf);
  state->regs.iofs = iofs;
  state->regs.ilen = ilen;
  state->regs.obuf = (char*) Bytes_val(obuf);
  state->regs.oofs = oofs;
  state->regs.olen = olen;

  while (!TZ_IS_BLOCKED(tz_operation_parser_step(state)));

  int read = ilen - state->regs.ilen;
  int written = olen - state->regs.olen;

  r = caml_alloc_tuple(3);
  Store_field(r, 0, Val_int(read));
  Store_field(r, 1, Val_int(written));

  switch (state->errno) {
  case TZ_BLO_DONE:
    Store_field(r, 2, Val_int(0));
    break;
  case TZ_BLO_FEED_ME:
    Store_field(r, 2, Val_int(1));
    break;
  case TZ_BLO_IM_FULL:
    Store_field(r, 2, Val_int(2));
    break;
  case TZ_ERR_INVALID_TAG:
    caml_failwith("operation_cparse_step: invalid tag");
  case TZ_ERR_INVALID_OP:
    caml_failwith("operation_cparse_step: invalid operation");
  case TZ_ERR_UNSUPPORTED:
    caml_failwith("operation_cparse_step: unsupported data");
  case TZ_ERR_TOO_LARGE:
    caml_failwith("operation_cparse_step: data size limitation exceeded");
  case TZ_ERR_TOO_DEEP:
    caml_failwith("operation_cparse_step: expression too deep");
  case TZ_ERR_INVALID_STATE:
    caml_failwith("operation_cparse_step: invalid state");
  default:
    char err[100];
    snprintf(err, 99, "operation_cparse_step: unknown error code %d", state->errno);
    caml_failwith(err);
  }
  CAMLreturn(r);
}
