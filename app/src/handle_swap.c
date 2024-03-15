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

#include <format.h>
#include <io.h>
#include <swap.h>

#include "handle_swap.h"
#include "keys.h"
#include "utils.h"

#include "parser/num_parser.h"

// based on app-exchange
#define TICKER           "XTZ"
#define ADDRESS_MAX_SIZE 63
/* the smallest unit is microtez */
#define DECIMALS         6

/* Check check_address_parameters_t.address_to_check against specified
 * parameters.
 *
 * Must set params.result to 0 on error, 1 otherwise */
void
swap_handle_check_address(check_address_parameters_t *params)
{
    TZ_PREAMBLE(("params=%p", params));

    if (params->address_to_check == NULL) {
        PRINTF("[ERROR] Address to check is null\n");
        goto bail;
    }

    if ((params->address_parameters_length == 0)
        || (params->address_parameters == NULL)) {
        PRINTF("[ERROR] Address parameters is null\n");
        goto bail;
    }

    char address[TZ_CAPTURE_BUFFER_SIZE] = {0};

    // Always tz1
    derivation_type_t derivation_type = DERIVATION_TYPE_ED25519;
    bip32_path_t      bip32_path;

    TZ_LIB_CHECK(read_bip32_path(&bip32_path, params->address_parameters,
                                 params->address_parameters_length));
    cx_ecfp_public_key_t pubkey;
    TZ_LIB_CHECK(derive_pk(&pubkey, derivation_type, &bip32_path));
    TZ_LIB_CHECK(
        derive_pkh(&pubkey, derivation_type, address, sizeof(address)));
    if (strcmp(params->address_to_check, address) != 0) {
        PRINTF("[ERROR] Check address fail: %s !=  %s\n",
               params->address_to_check, address);
        goto bail;
    }

    params->result = 1;
    FUNC_LEAVE();
    return;
end:
bail:
    params->result = 0;
    FUNC_LEAVE();
}

/* Format printable amount including the ticker from specified parameters.
 *
 * Must set empty printable_amount on error, printable amount otherwise */
void
swap_handle_get_printable_amount(get_printable_amount_parameters_t *params)
{
    FUNC_ENTER(("params=%p", params));

    uint64_t amount;

    if (!swap_str_to_u64(params->amount, params->amount_length, &amount)) {
        PRINTF("[ERROR] Fail to parse amount\n");
        goto error;
    }

    if (!format_fpu64_trimmed(params->printable_amount,
                              sizeof(params->printable_amount), amount,
                              DECIMALS)) {
        PRINTF("[ERROR] Fail to print amount\n");
        goto error;
    }

    strlcat(params->printable_amount, " ", sizeof(params->printable_amount));
    strlcat(params->printable_amount, TICKER,
            sizeof(params->printable_amount));

    FUNC_LEAVE();
    return;

error:
    memset(params->printable_amount, '\0', sizeof(params->printable_amount));
    FUNC_LEAVE();
}

typedef struct {
    uint64_t amount;
    uint64_t fee;
    char     destination_address[ADDRESS_MAX_SIZE];
} swap_transaction_parameters_t;

static swap_transaction_parameters_t G_swap_params;

static uint8_t *G_swap_transaction_result;

/* Backup up transaction parameters and wipe BSS to avoid collusion with
 * app-exchange BSS data.
 *
 * return false on error, true otherwise */
bool
swap_copy_transaction_parameters(create_transaction_parameters_t *params)
{
    FUNC_ENTER(("params=%p", params));

    swap_transaction_parameters_t params_copy;
    memset(&params_copy, 0, sizeof(params_copy));

    if (!swap_str_to_u64(params->amount, params->amount_length,
                         &params_copy.amount)) {
        PRINTF("[ERROR] Fail to parse amount\n");
        goto error;
    }

    if (!swap_str_to_u64(params->fee_amount, params->fee_amount_length,
                         &params_copy.fee)) {
        PRINTF("[ERROR] Fail to parse fee\n");
        goto error;
    }

    if (params->destination_address == NULL) {
        PRINTF("[ERROR] Destination address is null\n");
        goto error;
    }

    strlcpy(params_copy.destination_address, params->destination_address,
            sizeof(params_copy.destination_address));
    if (params_copy
            .destination_address[sizeof(params_copy.destination_address) - 1]
        != '\0') {
        PRINTF("[ERROR] Fail to copy destination address\n");
        goto error;
    }

    os_explicit_zero_BSS_segment();

    G_swap_transaction_result = &params->result;

    memcpy(&G_swap_params, &params_copy, sizeof(params_copy));

    FUNC_LEAVE();
    return true;

error:
    FUNC_LEAVE();
    return false;
}

void
swap_check_validity(void)
{
    tz_operation_state *op
        = &global.keys.apdu.sign.u.clear.parser_state.operation;
    char dstaddr[ADDRESS_MAX_SIZE];
    TZ_PREAMBLE((""));

    if (!G_called_from_swap) {
        TZ_SUCCEED();
    }

    if (G_swap_response_ready) {
        os_sched_exit(-1);
    }
    G_swap_response_ready = true;

    PRINTF("[DEBUG] batch_index = %u, nb_reveal=%d, tag=%d\n",
           op->batch_index, op->nb_reveal, op->last_tag);
    TZ_ASSERT(EXC_REJECT, op->nb_reveal <= 1);
    TZ_ASSERT(EXC_REJECT, (op->batch_index - op->nb_reveal) == 1);
    TZ_ASSERT(EXC_REJECT, op->last_tag == TZ_OPERATION_TAG_TRANSACTION);
    TZ_ASSERT(EXC_REJECT, op->last_amount == G_swap_params.amount);
    TZ_ASSERT(EXC_REJECT, op->last_fee == G_swap_params.fee);

    tz_format_address(op->destination, 22, dstaddr, sizeof(dstaddr));

    PRINTF("[DEBUG] dstaddr=\"%s\"\n", dstaddr);
    PRINTF("[DEBUG] G...dstaddr=\"%s\"\n", G_swap_params.destination_address);
    TZ_ASSERT(EXC_REJECT,
              !strcmp(dstaddr, G_swap_params.destination_address));

    TZ_POSTAMBLE;
}

/* Set create_transaction.result and call os_lib_end().
 *
 * Doesn't return */
void __attribute__((noreturn))
swap_finalize_exchange_sign_transaction(bool is_success)
{
    *G_swap_transaction_result = is_success;
    os_lib_end();
}

#else   // HAVE_SWAP
void
swap_check_validity(void)
{
}
#endif  // HAVE_SWAP
