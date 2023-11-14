#!/usr/bin/env python3
# Copyright 2023 Functori <contact@functori.com>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from utils.apdu import *
from utils.app import *

# original bytes : 0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316

if __name__ == "__main__":
    test_name = Path(__file__).stem

    def make_path(name: str) -> Path:
        return Path(test_name) / name

    with nano_app() as app:

        app.assert_screen(Screen.Home)

        app.parsing_error_signing(DEFAULT_ACCOUNT,
                                  "0100000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316",
                                  with_hash=True,
                                  text="INVALID_TAG",
                                  path=make_path("unknown_magic_bytes"))

        app.assert_screen(Screen.Home)

        app.parsing_error_signing(DEFAULT_ACCOUNT,
                                  "03000000000000000000000000000000000000000000000000000000000000000001016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316",
                                  with_hash=True,
                                  text="INVALID_TAG",
                                  path=make_path("unknown_operation"))

        app.assert_screen(Screen.Home)

        app.parsing_error_signing(DEFAULT_ACCOUNT,
                                  "0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e010000000000000000000000000000000000000000ff02000000020316",
                                  with_hash=True,
                                  text="INVALID_TAG",
                                  path=make_path("one_byte_removed_inside"))

        app.assert_screen(Screen.Home)

        app.parsing_error_signing(DEFAULT_ACCOUNT,
                                  "0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff0200000002031645",
                                  with_hash=True,
                                  text="INVALID_TAG",
                                  path=make_path("one_byte_added_at_the_end"))

        app.assert_screen(Screen.Home)

        app.parsing_error_signing(DEFAULT_ACCOUNT,
                                  "0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e0100000000000000000000000000000000000000000000ff02000000020316",
                                  with_hash=True,
                                  text="INVALID_TAG",
                                  path=make_path("one_byte_added_inside"))

        app.assert_screen(Screen.Home)

        app.parsing_error_signing(DEFAULT_ACCOUNT,
                                  "0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e0100000000000000000000000000000000000000000000ff02000000020316",
                                  with_hash=True,
                                  text="INVALID_TAG",
                                  path=make_path("one_byte_added_inside"))

        app.assert_screen(Screen.Home)

        app.hard_failing_signing(DEFAULT_ACCOUNT,
                                 "030000000000000000000000000000000000000000000000000000000000000000ce00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000c63966303966323935326433343532386337333366393436313563666333396263353535363139666335353064643461363762613232303863653865383637616133643133613665663939646662653332633639373461613961323135306432316563613239633333343965353963313362393038316631",
                                 with_hash=True,
                                 status_code=StatusCode.UNEXPECTED_SIGN_STATE,
                                 path=make_path("wrong_last_packet"))

        app.quit()
