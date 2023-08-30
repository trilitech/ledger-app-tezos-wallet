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
#include <swap.h>

#include "keys.h"
#include "utils.h"

#include "parser/num_parser.h"

#define TICKER_MAX_SIZE 9  // based on app-exchange

/* Check check_address_parameters_t.address_to_check against specified
 * parameters.
 *
 * Must set params.result to 0 on error, 1 otherwise */
void
swap_handle_check_address(check_address_parameters_t *params)
{
    FUNC_ENTER(("params=%p", params));

    if (params->address_to_check == NULL) {
        PRINTF("[ERROR] Address to check is null\n");
        goto error;
    }

    if (params->address_parameters_length == 0
        || params->address_parameters == NULL) {
        PRINTF("[ERROR] Address parameters is null\n");
        goto error;
    }

    if (params->extra_id_to_check == NULL) {
        PRINTF("[ERROR] Extra id is null\n");
        goto error;
    }

    if (params->extra_id_to_check[0] != '\0') {
        PRINTF("[ERROR] Extra id is not empty: %s\n",
               params->extra_id_to_check);
        goto error;
    }

    char address[TZ_CAPTURE_BUFFER_SIZE] = {0};

    // Always tz1
    derivation_type_t derivation_type = DERIVATION_TYPE_ED25519;
    bip32_path_t      bip32_path;

    TZ_CHECK(read_bip32_path(&bip32_path, params->address_parameters,
                             params->address_parameters_length));

    TZ_CHECK(
        derive_pkh(derivation_type, &bip32_path, address, sizeof(address)));

    if (strcmp(params->address_to_check, address) != 0) {
        PRINTF("[ERROR] Check address fail: %s !=  %s\n",
               params->address_to_check, address);
        goto error;
    }

    params->result = 1;
    FUNC_LEAVE();
    return;
error:
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
    uint8_t  decimals;
    char     ticker[TICKER_MAX_SIZE];

    if (!swap_parse_config(params->coin_configuration,
                           params->coin_configuration_length, ticker,
                           sizeof(ticker), &decimals)) {
        PRINTF("[ERROR] Fail to parse config\n");
        goto error;
    }

    if (!swap_str_to_u64(params->amount, params->amount_length, &amount)) {
        PRINTF("[ERROR] Fail to parse amount\n");
        goto error;
    }

    if (!format_fpu64_trimmed(params->printable_amount,
                              sizeof(params->printable_amount), amount,
                              decimals)) {
        PRINTF("[ERROR] Fail to print amount\n");
        goto error;
    }

    strlcat(params->printable_amount, " ", sizeof(params->printable_amount));
    strlcat(params->printable_amount, ticker,
            sizeof(params->printable_amount));

    FUNC_LEAVE();
    return;

error:
    memset(params->printable_amount, '\0', sizeof(params->printable_amount));
    FUNC_LEAVE();
}

#endif  // HAVE_SWAP
