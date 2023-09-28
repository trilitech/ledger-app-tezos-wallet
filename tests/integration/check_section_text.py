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
import os
import requests
from dataclasses import dataclass
import time

TIMEOUT = int(os.environ.get('TIMEOUT', '100'))

@dataclass
class Screen:
  title: str
  text: list[str]

  def matches(self, content: str, content_lines: int, device) -> bool:
    for l in self.text:
      l = device_alter_content(device, l)
      content = content.lstrip('\n')
      if len(l) == 0 or not content.startswith(l):
        return False
      content = content.removeprefix(l)

    return True

  def strip(self, content: str) -> str:
    for l in self.text:
      content = content.lstrip('\n')
      content = content.removeprefix(l)
    content = content.lstrip('\n')
    return content

def with_retry(url, f, timeout=TIMEOUT):
    attempts = timeout / 0.5
    while True:
        try:
            on_screen = get_screen(url)
            return f(on_screen)
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

    assert len(lines) > 0, "Unexpected empty screen. Speculos killed?"

    screen = Screen(title=lines[0], text=lines[1:])
    print(f'- {screen} -')
    return screen

def press_button(url, button):
    """Press a button on the ledger device."""
    print("- press", button)
    r = requests.post(f'{url}/button/{button}', json = {"action": "press-and-release"})
    r.raise_for_status()

def press_right(url):
    """Press the right button."""
    return press_button(url, 'right')

def press_left(url):
    """Press the left button."""
    return press_button(url, 'left')

def check_multi_screen(url, title, content, content_lines, device):
    """Assert that the screen contents across all screens with the given title match expected content."""
    while True:
      def check_screen(screen):
        assert screen.title == title, f"expected section '{title}' but on '{screen.title}'"
        assert screen.matches(content, content_lines, device), f"{screen} did not match {content}"
        return screen

      on_screen = with_retry(url, check_screen)
      content = on_screen.strip(content)

      if content == "":
        break

      press_right(url)

    print(f'- final screen {on_screen} -')

def check_potential_remaining_screen(url, device, title, content):
    """Ignore the next blank screen on nanosp/nanox to avoid missing a potential terminal 'S'."""
    # OCR issue https://github.com/LedgerHQ/speculos/issues/204
    if device in ["nanosp", "nanox"] and content.endswith('S'):

      def check_screen(screen):
        if screen.title == title:
          assert all([l == "" for l in screen.text]), f"Expected blank screen (containing only 'S'), got {screen.text}"
        return screen

      while True:
        press_right(url)
        screen = with_retry(url, check_screen)
        if screen.title != title:
          break

      def check_back(screen):
        assert screen.title == title

      press_left(url)
      # wait until we are back on last page of content
      with_retry(url, check_back)

def device_content_lines(device: str) -> int:
  if device == "nanos":
    return 2
  if device in ["nanosp", "nanox"]:
    return 4

  raise ValueError(f"unsupported device '{device}'")

def device_alter_content(device: str, content: str) -> str:
  if device == "nanos":
    return content
  if device in ["nanosp", "nanox"]:
    # OCR issue https://github.com/LedgerHQ/speculos/issues/204
    content = content.replace('S', '')
    content = content.replace('I', 'l')
    return content

  raise ValueError(f"unsupported device '{device}'")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Check a section of pages contain the expected content.")
    parser.add_argument("-d", "--device", help="device type: nanos | nanosp", required=True)
    parser.add_argument("-u", "--url", help="SPECULOS_URL", required=True)
    parser.add_argument("-t", "--title", help="Section title", required=True)
    parser.add_argument("-e", "--expected-content", help="Expected content of section", required=True)
    args = parser.parse_args()

    content_lines = device_content_lines(args.device)
    content = device_alter_content(args.device, args.expected_content)

    check_multi_screen(args.url, args.title, content, content_lines, args.device)
    check_potential_remaining_screen(args.url, args.device, args.title, args.expected_content)
