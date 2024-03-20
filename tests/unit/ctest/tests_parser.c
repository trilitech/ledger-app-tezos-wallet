/* Copyright 2023 Functori <contact@functori.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include <stdlib.h>
#include "ctest.h"
#include "operation_parser.h"

CTEST_DATA(operation_parser)
{
    tz_parser_state *state;
    char            *obuf;
    size_t           olen;
    uint8_t         *ibuf;
    size_t           ilen;
    size_t           max_ilen;
    char            *str;
    size_t           str_len;
    size_t           str_ofs;
};

CTEST_SETUP(operation_parser)
{
    data->state = malloc(sizeof(tz_parser_state));
    memset(data->state, 0, sizeof(tz_parser_state));
    data->olen = 50;
    data->obuf = malloc(data->olen + 1);
    memset(data->obuf, 0, data->olen + 1);
    data->max_ilen = 235;
    data->ilen     = data->max_ilen;
    data->ibuf     = malloc(data->max_ilen);
    memset(data->ibuf, 0, data->max_ilen);
    tz_operation_parser_init(data->state, TZ_UNKNOWN_SIZE, false);
    tz_parser_refill(data->state, NULL, 0);
    tz_parser_flush(data->state, data->obuf, data->olen);
    data->str = NULL;
}

CTEST_TEARDOWN(operation_parser)
{
    free(data->state);
    free(data->obuf);
    free(data->ibuf);
}

void
refill(struct ctest_operation_parser_data *data)
{
    data->ilen = 0;
    while (
        ((data->str_ofs < data->str_len) || (data->ilen < data->max_ilen))
        && sscanf(data->str + data->str_ofs, "%2hhx", &data->ibuf[data->ilen])
               == 1) {
        data->str_ofs += 2;
        data->ilen++;
    }
}

typedef struct {
    const char *name;
    uint8_t     complex : 1;
    int         field_index;
} tz_fields_check;

static void
fill_data_str(struct ctest_operation_parser_data *data, char *str)
{
    size_t str_len = strlen(str) / 2;
    data->str      = str;
    data->str_len  = str_len;
    data->str_ofs  = 0;
}

static void
check_field_complexity(struct ctest_operation_parser_data *data, char *str,
                       const tz_fields_check *fields_check,
                       size_t                 fields_check_size)
{
    fill_data_str(data, str);
    size_t fields_check_len = fields_check_size / sizeof(tz_fields_check);

    tz_operation_parser_set_size(data->state, (uint16_t)data->str_len);

    tz_parser_state *st = data->state;

    bool   result       = true;
    size_t idx          = 0;
    bool   already_seen = false;

    while (true) {
        while (!TZ_IS_BLOCKED(tz_operation_parser_step(st))) {
            // Loop while the result is successful and not blocking
        }

        switch (st->errno) {
        case TZ_BLO_FEED_ME:
            refill(data);
            tz_parser_refill(data->state, data->ibuf, data->ilen);
            continue;

        case TZ_BLO_IM_FULL:
            if (already_seen
                && strstr(st->field_info.field_name, fields_check[idx].name)
                       == NULL) {
                idx++;
                ASSERT_LT((intmax_t)idx, (intmax_t)fields_check_len);
                already_seen = false;
            }
            if (strstr(st->field_info.field_name, fields_check[idx].name)
                != NULL) {
                if ((fields_check[idx].complex
                     != st->field_info.is_field_complex)
                    || (fields_check[idx].field_index
                        != st->field_info.field_index)) {
                    CTEST_LOG(
                        "%s:%d '%s' field expected to have complex: %s "
                        "index: %d but "
                        "got complex: %s index: %d",
                        __FILE__, __LINE__, st->field_info.field_name,
                        fields_check[idx].complex ? "true" : "false",
                        fields_check[idx].field_index,
                        st->field_info.is_field_complex ? "true" : "false",
                        st->field_info.field_index);
                    result = false;
                }
                already_seen = true;
            } else if (st->field_info.is_field_complex) {
                CTEST_LOG(
                    "%s:%d '%s' has not been defined as an operation field "
                    "and therefore must not be complex",
                    __FILE__, __LINE__, st->field_info.field_name);
                result = false;
            }
            tz_parser_flush(st, data->obuf, data->olen);
            continue;

        case TZ_BLO_DONE:
            if (fields_check_len != (idx + 1)) {
                CTEST_ERR(
                    "%s:%d all the field have not been seen, %d fields "
                    "expected but got %d seen",
                    __FILE__, __LINE__, (int)fields_check_len, (int)idx);
            }
            ASSERT_TRUE(result);
            break;

        default:
            CTEST_ERR("%s:%d parsing error: %s", __FILE__, __LINE__,
                      tz_parser_result_name(st->errno));
        }
        break;
    }
}

CTEST2(operation_parser, check_proposals_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "0500ffdd6102321bc251e4a5190ad5b12b251069d9b400000020000000400bcd7b"
          "2cadcd87ecb0d5c50330fb59feed7432bffecede8a09a2b86cfb33847b0bcd7b2c"
          "adcd87ecb0d5c50330fb59feed7432bffecede8a09a2b86dac301a2d";
    const tz_fields_check fields_check[] = {
        {"Source",   false, 1},
        {"Period",   false, 2},
        {"Proposal", false, 3},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_ballot_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "0600ffdd6102321bc251e4a5190ad5b12b251069d9b4000000200bcd7b2cadcd87"
          "ecb0d5c50330fb59feed7432bffecede8a09a2b86cfb33847b00";
    const tz_fields_check fields_check[] = {
        {"Source",   false, 1},
        {"Period",   false, 2},
        {"Proposal", false, 3},
        {"Ballot",   false, 4},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_failing_noop_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "11000000c639663039663239353264333435323863373333663934363135636663"
          "333962633535353631396663353530646434613637626132323038636538653836"
          "376161336431336136656639396466626533326336393734616139613231353064"
          "323165636132396333333439653539633133623930383166316331316234343061"
          "633464333435356465646265346565306465313561386166363230643463383632"
          "343764396431333264653162623664613233643566663964386466666461323262"
          "6139613834";
    const tz_fields_check fields_check[] = {
        {"Message", false, 1},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_reveal_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "6b00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400747884d9ab"
          "df16b3ab745158925f567e222f71225501826fa83347f6cbe9c393";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Public key",    false, 4},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_simple_transaction_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "6c00ffdd6102321bc251e4a5190ad5b12b251069d9b4a0c21e020304904e010000"
          "0000000000000000000000000000000000000000";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Amount",        false, 4},
        {"Destination",   false, 5},
 //     {"Option",        _,     6},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_transaction_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "6c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e0100"
          "0000000000000000000000000000000000000000ff02000000020316";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Amount",        false, 4},
        {"Destination",   false, 5},
 //     {"Option",        _,     6},
  //    {"Tuple",         _,     7},
        {"Entrypoint",    false, 8},
        {"Parameter",     true,  9},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_double_transaction_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "6c00ffdd6102321bc251e4a5190ad5b12b251069d9b4a0c21e020304904e010000"
          "0000000000000000000000000000000000000000"
          "6c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e0100"
          "0000000000000000000000000000000000000000ff02000000020316";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1 },
        {"Fee",           false, 2 },
        {"Storage limit", false, 3 },
        {"Amount",        false, 4 },
        {"Destination",   false, 5 },
 //     {"Option",        _,     6 },
        {"Source",        false, 7 },
        {"Fee",           false, 8 },
        {"Storage limit", false, 9 },
        {"Amount",        false, 10},
        {"Destination",   false, 11},
 //     {"Option",        _,     12},
  //    {"Tuple",         _,     13},
        {"Entrypoint",    false, 14},
        {"Parameter",     true,  15},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_origination_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "6d00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304a0c21e000000"
          "0002037a0000000a07650100000001310002";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Balance",       false, 4},
        {"Delegate",      false, 5}, // None
        {"Code",          true,  6},
        {"Storage",       true,  7},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_delegation_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "6e01774d99da021b92d8c3dfc2e814c7658440319be2c09a0cf40509f906ff0059"
          "1e842444265757d6a65e3670ca18b5e662f9c0";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
 //     {"Option",        _,     4},
        {"Delegate",      false, 5},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_register_global_constant_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "6f00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040000000a0707"
          "0100000001310002";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Value",         true,  4},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_set_deposit_limit_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "70027c252d3806e6519ed064026bdb98edf866117331e0d40304f80204ffa09c0"
          "1";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
 //     {"Option",        _,     4},
        {"Staking limit", false, 5},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_increase_paid_storage_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "7100ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304050100000000"
          "0000000000000000000000000000000000";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Amount",        false, 4},
        {"Destination",   false, 5},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_set_consensus_key_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "7200c921d4487c90b4472da6cc566a58d79f0d991dbf904e02030400747884d9ab"
          "df16b3ab745158925f567e222f71225501826fa83347f6cbe9c393";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Public key",    false, 4},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_transfer_ticket_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "9e00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000002037a"
          "0000000a076501000000013100020000ffdd6102321bc251e4a5190ad5b12b2510"
          "69d9b4010100000000000000000000000000000000000000000000000007646566"
          "61756c74";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Contents",      true,  4},
        {"Type",          true,  5},
        {"Ticketer",      false, 6},
        {"Amount",        false, 7},
        {"Destination",   false, 8},
        {"Entrypoint",    false, 9},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_sc_rollup_add_messages_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "c900ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304000000140000"
          "000301234500000001670000000489abcdef";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Message",       false, 4},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_sc_rollup_execute_outbox_message_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "ce00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304000000000000"
          "000000000000000000000000000000000000000000000000000000000000000000"
          "00000000000000000000000000000000c639663039663239353264333435323863"
          "373333663934363135636663333962633535353631396663353530646434613637"
          "626132323038636538653836376161336431336136656639396466626533326336"
          "393734616139613231353064323165636132396333333439653539633133623930"
          "383166316331316234343061633464333435356465646265346565306465313561"
          "386166363230643463383632343764396431333264653162623664613233643566"
          "6639643864666664613232626139613834";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Rollup",        false, 4},
        {"Commitment",    false, 5},
        {"Output proof",  true,  6},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}

CTEST2(operation_parser, check_sc_rollup_originate_complexity)
{
    char str[]
        = "030000000000000000000000000000000000000000000000000000000000000000"
          "c800ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000000c639"
          "663039663239353264333435323863373333663934363135636663333962633535"
          "353631396663353530646434613637626132323038636538653836376161336431"
          "336136656639396466626533326336393734616139613231353064323165636132"
          "396333333439653539633133623930383166316331316234343061633464333435"
          "356465646265346565306465313561386166363230643463383632343764396431"
          "333264653162623664613233643566663964386466666461323262613961383400"
          "00000a07070100000001310002ff0000003f00ffdd6102321bc251e4a5190ad5b1"
          "2b251069d9b401f6552df4f5ff51c3d13347cab045cfdb8b9bd8030278eb8b6ab9"
          "a768579cd5146b480789650c83f28e";
    const tz_fields_check fields_check[] = {
        {"Source",        false, 1},
        {"Fee",           false, 2},
        {"Storage limit", false, 3},
        {"Kind",          false, 4},
        {"Kernel",        true,  5},
        {"Parameters",    true,  6},
 //     {"Option",        _,     7},
        {"Whitelist",     false, 8},
    };
    check_field_complexity(data, str, fields_check, sizeof(fields_check));
}
