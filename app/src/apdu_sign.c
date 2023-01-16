#include "cx.h"

#include "apdu.h"
#include "globals.h"
#include "keys.h"
#include "memory.h"
#include "ui.h"
#include "ui_stream.h"

#include "parser/parser_state.h"
#include "parser/operation_parser.h"

#include <string.h>
#include <stdbool.h>

#define P1_FIRST          0x00
#define P1_NEXT           0x01
#define P1_HASH_ONLY_NEXT 0x03  // You only need it once
#define P1_LAST_MARKER    0x80

static inline void clear_data(void) {
    memset(&global.apdu.hash, 0, sizeof(global.apdu.hash));
    memset(&global.apdu.sign, 0, sizeof(global.apdu.sign));
}

typedef struct {
  uint8_t cla;
  uint8_t ins;
  uint8_t p1;
  uint8_t p2;
  uint8_t buff_size;
  uint8_t *buff;
  bool is_last;
  bool is_first;
} packet_t;

static void init_packet(packet_t *pkt) {
  pkt->cla = G_io_apdu_buffer[OFFSET_CLA];
  pkt->ins = G_io_apdu_buffer[OFFSET_INS];
  pkt->p1 = G_io_apdu_buffer[OFFSET_P1];
  pkt->p2 = G_io_apdu_buffer[OFFSET_P2];
  pkt->buff_size = G_io_apdu_buffer[OFFSET_LC];
  pkt->buff = &G_io_apdu_buffer[OFFSET_CDATA];
  pkt->is_last = pkt->p1 & P1_LAST_MARKER;
  pkt->is_first = (pkt->p1 & ~P1_LAST_MARKER) == 0;
  if (pkt->buff_size > MAX_APDU_SIZE) THROW(EXC_WRONG_LENGTH_FOR_INS);
}

static int write_signature(uint8_t *const out, uint8_t const *const data, size_t const data_length) {

    key_pair_t key_pair = {0};
    size_t signature_size = 0;

    int error = generate_key_pair(&key_pair,
                                  global.path_with_curve.derivation_type,
                                  &global.path_with_curve.bip32_path);
    if (error) {
        THROW(EXC_WRONG_VALUES);
    }

    error = 0;
    BEGIN_TRY {
        TRY {
            signature_size = sign(out,
                                  MAX_SIGNATURE_SIZE,
                                  global.path_with_curve.derivation_type,
                                  &key_pair,
                                  data,
                                  data_length);
        }
        CATCH_OTHER(e) {
            error = e;
        }
        FINALLY {
            memset(&key_pair, 0, sizeof(key_pair));
        }
    }
    END_TRY;

    if (error) {
        THROW(error);
    }

    return signature_size;
}

static void sign_packet() {
  size_t tx = 0;
  bool send_hash = true; // TODO change this when we handle more APDU instructions

  if (send_hash) {
    memcpy(&G_io_apdu_buffer[tx],
           global.apdu.hash.final_hash,
           sizeof(global.apdu.hash.final_hash));
    tx += sizeof(global.apdu.hash.final_hash);
  }

  tx += write_signature(&G_io_apdu_buffer[tx],
                        global.apdu.hash.final_hash,
                        sizeof(global.apdu.hash.final_hash));
  tx = finalize_successful_send(tx);

  if (global.apdu.sign.step != SIGN_ST_WAIT_USER_INPUT) THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (!(global.apdu.sign.received_last_msg)) THROW(EXC_UNEXPECTED_SIGN_STATE);

  delayed_send(tx);

  global.step = ST_IDLE;
  global.apdu.sign.step = SIGN_ST_IDLE;
}

static void send_reject() {
  if (global.apdu.sign.step != SIGN_ST_WAIT_USER_INPUT) THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (!(global.apdu.sign.received_last_msg)) THROW(EXC_UNEXPECTED_SIGN_STATE);
  clear_data();
  delay_reject();

  global.step = ST_IDLE;
  global.apdu.sign.step = SIGN_ST_IDLE;
}

static void send_continue() {
  if (global.apdu.sign.step != SIGN_ST_WAIT_USER_INPUT) THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (global.apdu.sign.received_last_msg) THROW(EXC_UNEXPECTED_SIGN_STATE);
  delayed_send(finalize_successful_send(0));
  global.apdu.sign.step = SIGN_ST_WAIT_DATA;
}

static void refill() {
  tz_parser_regs *regs = &global.apdu.sign.parser_regs;
  while(!TZ_IS_BLOCKED(tz_operation_parser_step(&global.apdu.sign.parser_state, regs))){};
  PRINTF("[DEBUG] refill() \n");
  switch (global.apdu.sign.parser_state.errno) {
  case TZ_BLO_IM_FULL:
    strncpy (global.stream.buffer.title, global.apdu.sign.parser_state.field_name, TZ_UI_STREAM_TITLE_WIDTH);
    global.stream.buffer.value[regs->oofs] = 0;
    tz_ui_stream_push ();
    tz_parser_regs *regs = &global.apdu.sign.parser_regs;
    regs->olen = TZ_UI_STREAM_CONTENTS_WIDTH * TZ_UI_STREAM_CONTENTS_LINES;
    regs->oofs = 0;
    break;
  case TZ_BLO_FEED_ME:
    send_continue();
    break;
  case TZ_BLO_DONE:
    if (!(global.apdu.sign.received_last_msg) ||
        (global.apdu.sign.parser_regs.ilen != 0)) {
      failwith ("parsing done but some data left");
    }
    if(global.apdu.sign.parser_regs.oofs != 0) {
      strncpy (global.stream.buffer.title, global.apdu.sign.parser_state.field_name, TZ_UI_STREAM_TITLE_WIDTH);
      global.stream.buffer.value[global.apdu.sign.parser_regs.oofs] = 0;
      tz_ui_stream_push ();
    }
    tz_ui_stream_close ();
    break;
  default:
    failwith("parsing error");
  }
}

static void hash_block(blake2b_hash_state_t *const state, uint8_t *current) {
  if (!(state->initialized)) THROW(EXC_MEMORY_ERROR);
  cx_hash((cx_hash_t *) &state->state, 0, current, B2B_BLOCKBYTES, NULL, 0);
}

static void finish_hash(uint8_t *const out, size_t const out_size,
                        blake2b_hash_state_t *const state, uint8_t *buff,
                        size_t buff_length) {
  if (!(state->initialized)) THROW(EXC_MEMORY_ERROR);
  if (buff_length > B2B_BLOCKBYTES) THROW(EXC_MEMORY_ERROR);
  cx_hash((cx_hash_t *) &state->state, CX_LAST, buff, buff_length, out, out_size);
}

static void fill_message_data(packet_t *pkt, size_t *hash_offset) {
  uint8_t *data = pkt->buff + *hash_offset;
  size_t size = pkt->buff_size - *hash_offset;
  size_t copied = MIN(B2B_BLOCKBYTES - global.apdu.sign.message_data_length, size);
  memcpy(global.apdu.sign.message_data + global.apdu.sign.message_data_length,
         data, copied);
  global.apdu.sign.message_data_length += copied;
  *hash_offset += copied;
}

static void hash_packet(packet_t *pkt) {
  size_t hash_offset = 0;
  while(hash_offset < pkt->buff_size) {
    fill_message_data(pkt, &hash_offset);
    if (global.apdu.sign.message_data_length == B2B_BLOCKBYTES) {
      hash_block( &global.apdu.hash.hash_state, global.apdu.sign.message_data );
      global.apdu.sign.message_data_length = 0;
    }
  }
}

static size_t handle_first_apdu(packet_t *pkt) {

  if (global.apdu.sign.step != SIGN_ST_IDLE) THROW(EXC_UNEXPECTED_SIGN_STATE);

  clear_data();
  read_bip32_path(&global.path_with_curve.bip32_path, pkt->buff, pkt->buff_size);
  global.path_with_curve.derivation_type = parse_derivation_type(pkt->p2);

  // init hash
  cx_blake2b_init(&global.apdu.hash.hash_state.state, SIGN_HASH_SIZE * 8);
  global.apdu.hash.hash_state.initialized = true;

  tz_operation_parser_init(&global.apdu.sign.parser_state, TZ_UNKNOWN_SIZE, false);
  tz_parser_regs_refill (&global.apdu.sign.parser_regs, NULL, 0);
  tz_parser_regs_flush (&global.apdu.sign.parser_regs, global.stream.buffer.value, TZ_UI_STREAM_CONTENTS_SIZE);

  tz_ui_stream_init (refill, sign_packet, send_reject);

  global.apdu.sign.step = SIGN_ST_WAIT_DATA;
  return finalize_successful_send(0);
}

static size_t handle_data_apdu(packet_t *pkt) {
  if (global.apdu.sign.step != SIGN_ST_WAIT_DATA)
    // we received a packet while we did not explicitly asked for one
    THROW(EXC_UNEXPECTED_SIGN_STATE);
  if (global.apdu.sign.parser_regs.ilen > 0)
    // we asked for more input but did not consume what we already had
    THROW(EXC_UNEXPECTED_SIGN_STATE);
  global.apdu.sign.packet_index++; // XXX drop or check

  // do the incremental hashing
  hash_packet(pkt);
  global.apdu.sign.total_length += pkt->buff_size;
  if (pkt->is_last) {
    global.apdu.sign.received_last_msg = true;
    finish_hash(global.apdu.hash.final_hash,
                sizeof(global.apdu.hash.final_hash),
                &global.apdu.hash.hash_state,
                global.apdu.sign.message_data,
                global.apdu.sign.message_data_length);
  }

  // refill the parser's input
  tz_parser_regs_refill (&global.apdu.sign.parser_regs, pkt->buff, pkt->buff_size);
  if (pkt->is_last) {
    tz_operation_parser_set_size(&global.apdu.sign.parser_state, global.apdu.sign.total_length);
  }

  // loop getting and parsing packets until we have a first screen
  if (tz_ui_stream_current_screen_kind() == TZ_UI_STREAM_DISPLAY_INIT) {
    refill ();
    if (tz_ui_stream_current_screen_kind() == TZ_UI_STREAM_DISPLAY_INIT) {
      global.apdu.sign.step = SIGN_ST_WAIT_DATA;
      return finalize_successful_send(0);
    }
  }

  // launch parsing and UI (once we have a first screen)
  global.apdu.sign.step = SIGN_ST_WAIT_USER_INPUT;
  tz_ui_stream ();
}

size_t handle_apdu_sign(__attribute__((unused)) bool return_hash) {
  packet_t pkt;
  init_packet(&pkt);

  if (pkt.is_first) {
    if (global.step != ST_IDLE) THROW (EXC_UNEXPECTED_STATE);
    global.step = ST_SIGN;
    return handle_first_apdu(&pkt);
  } else {
    if (global.step != ST_SIGN) THROW (EXC_UNEXPECTED_STATE);
    return handle_data_apdu(&pkt);
  }
}
