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

#include "globals.h"

size_t tz_ui_stream_push(tz_ui_cb_type_t type, const char *title,
                         const char *value, tz_ui_icon_t icon) {
    return tz_ui_stream_pushl(type, title, value, -1, icon);
}

size_t tz_ui_stream_push_all(__attribute__((unused))tz_ui_cb_type_t type,
                             __attribute__((unused))const char *title,
                             const char *value,
                             __attribute__((unused))tz_ui_icon_t icon) {
    return strlen(value);
}

size_t tz_ui_stream_pushl(__attribute__((unused))tz_ui_cb_type_t type,
                          __attribute__((unused))const char *title,
                          const char *value, ssize_t max,
                          __attribute__((unused))tz_ui_icon_t icon) {
    size_t length = strlen(value);

    if (max != -1)
        length = MIN(length, (size_t)max);

    return length;
}

#endif
