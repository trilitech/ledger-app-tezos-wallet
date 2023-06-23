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

#include "globals.h"
#include "ui_stream_bagl.h"
#include "ui_stream_nbgl.h"

void tz_ui_stream_init (void (*cb)(tz_ui_cb_type_t)) {
  FUNC_ENTER(("cb=%p", cb));
  memset(&global.stream, 0x0, sizeof(global.stream));
  global.stream.cb = cb;
  global.stream.full = false;
  global.stream.current = -1;
  global.stream.total = -1;
  FUNC_LEAVE();
}

void tz_ui_stream_close () {
  FUNC_ENTER(("void"));
  if (global.stream.full) {
    failwith("trying to close already closed stream display");
  }
  global.stream.full = true;
  FUNC_LEAVE();
}

tz_ui_stream_screen_kind tz_ui_stream_current_screen_kind () {
  FUNC_ENTER(("void"));
  if (global.stream.current < 0)
    return TZ_UI_STREAM_DISPLAY_INIT;
  else if (global.stream.current == 0)
    return TZ_UI_STREAM_DISPLAY_FIRST;
  else if (global.stream.current == global.stream.total - TZ_UI_STREAM_HISTORY_SCREENS + 1)
    return TZ_UI_STREAM_DISPLAY_CANNOT_GO_BACK;
  else if (global.stream.current == global.stream.total + 1)
    return TZ_UI_STREAM_DISPLAY_ACCEPT;
  else if (global.stream.current == global.stream.total + 2)
    return TZ_UI_STREAM_DISPLAY_REJECT;
  else
    return TZ_UI_STREAM_DISPLAY_CONT;
  FUNC_LEAVE();
}

__attribute__((noreturn)) void tz_ui_stream() {
  FUNC_ENTER(("void"));
#ifdef HAVE_BAGL
  if (global.stream.pressed_right)
    succ();

  redisplay();
#endif
  THROW(ASYNC_EXCEPTION);
}
