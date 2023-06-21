/* Tezos Ledger application - Home screen display

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

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

#include "globals.h"
#include "ui_home_bagl.h"

/* Prototypes */




    }

}


  FUNC_ENTER(("void"));
  switch (global.home_screen) {
  case SCREEN_CLEAR_SIGN:
    break;
    break;
  }
  FUNC_LEAVE();
}

void ui_initial_screen(void) {
  FUNC_ENTER(("void"));
/*
 * XXXrcd: consider decision...
 *         should we revert to clear signing when
 *         we start again?
 */         
  global.home_screen = SCREEN_CLEAR_SIGN;
  tz_ui_home_redisplay();
  FUNC_LEAVE();
}
