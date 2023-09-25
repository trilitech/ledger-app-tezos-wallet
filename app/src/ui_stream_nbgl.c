/* Tezos Ledger application - Generic stream display

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
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

#ifdef HAVE_NBGL

#include <nbgl_use_case.h>
#include <ux.h>

#include "globals.h"

void
tz_reject(void)
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("void"));

    s->cb(TZ_UI_STREAM_CB_REJECT);
    ui_home_init();

    FUNC_LEAVE();
}

void
tz_reject_ui(void)
{
    // Stax can move into user input at any point in the flow
    global.apdu.sign.step              = SIGN_ST_WAIT_USER_INPUT;
    global.apdu.sign.received_last_msg = true;

    nbgl_useCaseStatus("Rejected", false, tz_reject);
}

void
tz_ui_start(void)
{
    FUNC_ENTER(("void"));

    FUNC_LEAVE();
    return;
}

void
tz_ui_stream_init(void (*cb)(uint8_t))
{
    tz_ui_stream_t *s = &global.stream;

    FUNC_ENTER(("cb=%p", cb));
    memset(s, 0x0, sizeof(*s));
    s->cb      = cb;
    s->full    = false;
    s->current = 0;
    s->total   = -1;

    nbgl_useCaseReviewStart(&C_tezos, "Review request to sign operation",
                            NULL, "Reject request", tz_ui_start,
                            tz_reject_ui);

    FUNC_LEAVE();
}

#endif
