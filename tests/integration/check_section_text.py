#!/usr/bin/env python3
# Copyright 2023 Trilitech <contact@trili.tech>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import requests
from dataclasses import dataclass
import time

MAX_ATTEMPTS = 200

@dataclass
class Screen:
  title: str
  text: str

def with_retry(f, message=None, attempts=MAX_ATTEMPTS):
    while True:
        try:
            return f()
        except AssertionError as e:
            print('- retrying:', e)
            if attempts <= 0:
                raise e
        attempts -= 1
        time.sleep(0.5)

def get_screen(url):
    """Retrieve the current screen title & contents"""
    r = requests.get(f'{url}/events', params = {"currentscreenonly":"true"})
    r.raise_for_status()
    lines = [e["text"] for e in r.json()["events"]]

    screen = Screen(title=lines[0], text=''.join(lines[1:]))
    print(f'- {screen} -')
    return screen

def press_button(url, button, attempts=MAX_ATTEMPTS):
    """Press a button on the ledger device."""
    old_screen = get_screen(url)
    print("- press", button)

    while attempts > 0:
        r = requests.post(f'{url}/button/{button}', json = {"action": "press-and-release"})
        r.raise_for_status()

        new_screen = get_screen(url)
        if not (old_screen == new_screen or (new_screen.text.strip() == "" and new_screen.title.strip() == "")):
            return new_screen

        attempts -= 1
        time.sleep(0.5)

    print(f"- pressed {button} did not advance screen. Old screen: {old_screen}")
    return old_screen

def press_left(url):
    """Press the left button."""
    return press_button(url, 'left')

def press_right(url):
    """Press the right button."""
    return press_button(url, 'right')

def press_both(url):
    """Press both button."""
    return press_button(url, 'both')

def check_single_screen(url, title, content, attempts=MAX_ATTEMPTS):
    """Assert that the content of the current screen matches the expected value, with retries."""
    expected = Screen(title, content)

    def check():
        on_screen = get_screen(url)
        assert on_screen == expected, f'expected {expected}, got {on_screen}'

    with_retry(check)

def check_multi_screen(url, title, content, attempts=MAX_ATTEMPTS):
    """Assert that the screen contents across all screens with the given title match expected content."""
    def check_title():
        on_screen = get_screen(url)
        assert title == on_screen.title, f'found title {on_screen.title}, expected {title}'
        return on_screen

    on_screen = with_retry(check_title)
    prev_text = ""

    print("Got title:", on_screen)

    while on_screen.title == title:
        try:
            assert content.startswith(on_screen.text), f'Expected to find "{on_screen.text}", but no match against "{content}"'
            content = content.removeprefix(on_screen.text)
            prev_text = on_screen.text
        except AssertionError:
            # Previous screen had not fully loaded
            up_to = content.index(on_screen.text)
            unloaded = content[:up_to]
            expected_prev = prev_text + unloaded
            print(f'- checking previous screen with "{expected_prev}"')
            press_left(url)
            check_single_screen(url, title, expected_prev)
            content = content.removeprefix(unloaded)
            prev_text = ""

        if content == "":
            break

        on_screen = press_right(url)

    assert content == "", f'Remaining content "{content}" not found on screen "{on_screen}".'
    # End of final screen of section

    print(f'- final screen {on_screen} -')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Check a section of pages contain the expected content.")
    parser.add_argument("-u", "--url", help="SPECULOS_URL", required=True)
    parser.add_argument("-t", "--title", help="Section title", required=True)
    parser.add_argument("-e", "--expected-content", help="Expected content of section", required=True)
    parser.add_argument("-m", "--multi", type=bool, default=False, help="Could content be split over multiple pages.")
    args = parser.parse_args()

    if not args.multi:
        check_single_screen(args.url, args.title, args.expected_content)
    else:
        check_multi_screen(args.url, args.title, args.expected_content)
