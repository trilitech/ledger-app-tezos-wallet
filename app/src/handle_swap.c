/* Tezos Ledger application - Swap requirement

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 TriliTech <contact@trili.tech>
   Copyright 2023 Functori <contact@functori.com>

   With code excerpts from:
   - Legacy Tezos app, Copyright 2019 Obsidian Systems
   - Ledger Blue sample apps, Copyright 2016 Ledger

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#ifdef HAVE_SWAP

#include "handle_swap.h"
#include "keys.h"
#include "swap.h"
#include "utils.h"

#include "parser/num_parser.h"

// based on app-exchange
#define TICKER_MAX_SIZE 9
#define ADDRESS_MAX_SIZE 63

/* Check check_address_parameters_t.address_to_check against specified
 * parameters.
 *
 * Must set params.result to 0 on error, 1 otherwise */
void swap_handle_check_address(check_address_parameters_t *params) {
  FUNC_ENTER(("params=%p", params));

  tz_err_t error = TZ_OK;

  if (params->address_to_check == NULL) {
    PRINTF("[DEBUG] Address to check is null\n");
    goto error;
  }

  if (params->address_parameters_length == 0 ||
      params->address_parameters == NULL) {
    PRINTF("[DEBUG] Address parameters is null\n");
    goto error;
  }

  if (params->extra_id_to_check == NULL) {
    PRINTF("[DEBUG] Extra id is null\n");
    goto error;
  }

  if (params->extra_id_to_check[0] != '\0') {
    PRINTF("[DEBUG] Extra id is not empty: %s\n", params->extra_id_to_check);
    goto error;
  }

  char address[TZ_CAPTURE_BUFFER_SIZE] = {0};

  // Always tz1 ?
  derivation_type_t derivation_type = DERIVATION_TYPE_ED25519;
  bip32_path_t bip32_path;

  TZ_CHECK(read_bip32_path(&bip32_path,
			   params->address_parameters,
			   params->address_parameters_length));

  TZ_CHECK(print_pkh(bip32_path, derivation_type, address));

  if (strcmp(params->address_to_check, address) != 0) {
    PRINTF("[DEBUG] Check address fail: %s !=  %s\n", params->address_to_check, address);
    goto error;
  }

  params->result = 1;
  FUNC_LEAVE();
  return;
 error:
 end:
  params->result = 0;
  FUNC_LEAVE();
}

bool swap_u64_to_printable_amount(uint64_t amount,
                                  uint8_t decimals,
                                  char *ticker,
                                  char *out,
                                  uint8_t out_len) {

  if (!tz_print_uint64(amount,
                       out,
                       out_len,
                       decimals))
    return false;

  strlcat(out, " ", out_len);
  strlcat(out, ticker, out_len);

  return true;
}

/* Format printable amount including the ticker from specified parameters.
 *
 * Must set empty printable_amount on error, printable amount otherwise */
void swap_handle_get_printable_amount(get_printable_amount_parameters_t *params) {
  FUNC_ENTER(("params=%p", params));

  uint64_t amount;
  uint8_t decimals;
  char ticker[TICKER_MAX_SIZE];

  if (!swap_parse_config(params->coin_configuration,
                         params->coin_configuration_length,
                         ticker,
                         sizeof(ticker),
                         &decimals)) {
    PRINTF("[DEBUG] Fail to parse config\n");
    goto error;
  }

  if (!swap_str_to_u64(params->amount, params->amount_length, &amount)) {
    PRINTF("[DEBUG] Fail to parse amount\n");
    goto error;
  }

  if (!swap_u64_to_printable_amount(amount,
                                    decimals,
                                    ticker,
                                    params->printable_amount,
                                    sizeof(params->printable_amount))) {
    PRINTF("[DEBUG] Fail to print amount\n");
    goto error;
  }

  FUNC_LEAVE();
  return;

 error:
  memset(params->printable_amount, '\0', sizeof(params->printable_amount));
  FUNC_LEAVE();
}

typedef struct {
  bool initialized;
  uint8_t decimals;
  char ticker[TICKER_MAX_SIZE];
  uint64_t amount;
  uint64_t fee;
  char destination_address[ADDRESS_MAX_SIZE];
} swap_transaction_parameters_t;

static swap_transaction_parameters_t G_swap_transaction_parameters;

static uint8_t* G_swap_transaction_result;

/* Backup up transaction parameters and wipe BSS to avoid collusion with
 * app-exchange BSS data.
 *
 * return false on error, true otherwise */
bool swap_copy_transaction_parameters(create_transaction_parameters_t *params) {
  FUNC_ENTER(("params=%p", params));

  swap_transaction_parameters_t params_copy;
  memset(&params_copy, 0, sizeof(params_copy));

  if (!swap_parse_config(params->coin_configuration,
                         params->coin_configuration_length,
                         params_copy.ticker,
                         sizeof(params_copy.ticker),
                         &params_copy.decimals)) {
    PRINTF("[DEBUG] Fail to parse config\n");
    goto error;
  }

  if (!swap_str_to_u64(params->amount,
                       params->amount_length,
                       &params_copy.amount)) {
    PRINTF("[DEBUG] Fail to parse amount\n");
    goto error;
  }

  if (!swap_str_to_u64(params->fee_amount,
                       params->fee_amount_length,
                       &params_copy.fee)) {
    PRINTF("[DEBUG] Fail to parse fee\n");
    goto error;
  }

  strlcpy(params_copy.destination_address,
          params->destination_address,
          sizeof(params_copy.destination_address));
  if (params_copy.destination_address[sizeof(params_copy.destination_address) - 1] != '\0') {
    PRINTF("[DEBUG] Fail to copy destination address\n");
    goto error;
  }

  if (params->destination_address_extra_id == NULL) {
    PRINTF("[DEBUG] Destination address extra id is null\n");
    goto error;
  }

  if (params->destination_address_extra_id[0] != '\0') {
    PRINTF("[DEBUG] Destination address extra id is not empty: %s\n", params->destination_address_extra_id);
    goto error;
  }

  params_copy.initialized = true;

  os_explicit_zero_BSS_segment();

  G_swap_transaction_result = &params->result;

  memcpy(&G_swap_transaction_parameters, &params_copy, sizeof(params_copy));

  FUNC_LEAVE();
  return true;

 error:
  FUNC_LEAVE();
  return false;
}

bool swap_check_validity(uint8_t *destination,
                         uint16_t batch_size,
                         tz_operation_tag tag,
                         char *fee,
                         char *amount) {
  FUNC_ENTER(("destination=%p, batch_size=%u, tag=%d, fee=%p, amount=%p", destination, batch_size, tag, fee, amount));

  char buff[ADDRESS_MAX_SIZE]; // max(ADDRESS_MAX_SIZE = 63, MAX_PRINTABLE_AMOUNT_SIZE = 50)

  if (!G_swap_transaction_parameters.initialized) {
    PRINTF("[DEBUG] Transaction_parameters non initialized\n");
    goto error;
  }

  if (batch_size != 1) {
    PRINTF("[DEBUG] More than one operation parsed: %d\n", batch_size);
    goto error;
  }

  if (tag != TZ_OPERATION_TAG_TRANSACTION) {
    PRINTF("[DEBUG] The operation parsed is not a transaction: %d\n", tag);
    goto error;
  }

  tz_format_address(destination, 22, buff);
  if (strcmp(buff, G_swap_transaction_parameters.destination_address) != 0) {
    PRINTF("[DEBUG] Different address: %s !=  %s\n", buff, G_swap_transaction_parameters.destination_address);
    goto error;
  }

  if (!swap_u64_to_printable_amount(G_swap_transaction_parameters.amount,
                                    G_swap_transaction_parameters.decimals,
                                    G_swap_transaction_parameters.ticker,
                                    buff,
                                    sizeof(buff))) {
    PRINTF("[DEBUG] Fail to print amount\n");
    goto error;
  }

  if (strcmp(buff, amount) != 0) {
    PRINTF("[DEBUG] Different amount: %d != %d\n", buff, amount);
    goto error;
  }

  if (!swap_u64_to_printable_amount(G_swap_transaction_parameters.fee,
                                    G_swap_transaction_parameters.decimals,
                                    G_swap_transaction_parameters.ticker,
                                    buff,
                                    sizeof(buff))) {
    PRINTF("[DEBUG] Fail to print fee\n");
    goto error;
  }

  if (strcmp(buff, fee) != 0) {
    PRINTF("[DEBUG] Different fee: %d != %d\n", buff, fee);
    goto error;
  }

  FUNC_LEAVE();
  return true;

 error:
  FUNC_LEAVE();
  return false;
}

/* Set create_transaction.result and call os_lib_end().
 *
 * Doesn't return */
void __attribute__((noreturn)) swap_finalize_exchange_sign_transaction(bool is_success) {
  *G_swap_transaction_result = is_success;
  os_lib_end();
}

#endif  // HAVE_SWAP
