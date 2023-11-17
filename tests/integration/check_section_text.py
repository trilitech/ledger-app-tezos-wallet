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

TIMEOUT = int(os.environ.get('TIMEOUT', '5'))

@dataclass
class Screen:
  title: str
  text: list[str]

  def __str__(self):
    return f"title=\"{self.title}\" Text=[{', '.join(self.text)}]"

  def matches(self, content: str, content_lines: int) -> bool:
    for l in self.text:
      content = content.lstrip('\n')
      print(f"l: {l}, c: {content[:len(l)]}")
      if not content.startswith(l):
        return False
      content = content.removeprefix(l)
      content = content.lstrip('\n')

    return True

  def strip(self, content: str) -> str:
    for l in self.text:
      content = content.lstrip('\n')
      content = content.removeprefix(l)
    content = content.lstrip('\n')
    return content

def with_retry(f, attempts= (2 * TIMEOUT)):
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

    assert len(lines) > 0, "Unexpected empty screen. Speculos killed?"

    screen = Screen(title=lines[0], text=lines[1:])
    print(f'- {screen} -')
    return screen

def find_last(lst, elm):
  """Find the index of the last element in lst that matches elm"""
  gen = (len(lst) - 1 - i for i, v in enumerate(reversed(lst)) if v == elm)
  return next(gen, None)

def get_titled_screen(url, title):
    """Retrieve the screen until title"""
    r = requests.get(f'{url}/events')
    r.raise_for_status()
    lines = [e["text"] for e in r.json()["events"]]

    assert len(lines) > 0, "Unexpected empty screen. Speculos killed?"

    assert title in lines, f"Title \"{title}\" no found"

    title_index = find_last(lines, title)

    screen = Screen(title=lines[title_index], text=lines[title_index+1:])
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
      def check_screen():
        if device in ["nanos", "nanosp"]:
          screen = get_titled_screen(url, title)
          assert screen.title == title, f"expected section '{title}' but on '{screen.title}'"
        # https://github.com/trilitech/ledger-app-tezos-wallet/issues/43
        # Get screens contents with the 'events' service for nanox
        # while the 'events?currentscreenonly=true' service does not
        # work properly for nanox in the sha-6a34680 version of
        # speculos
        if device == "nanox":
          screen = get_titled_screen(url, title)
        assert screen.matches(content, content_lines), \
               f"{screen} did not match {content[:10]}...{content[-10:]}"

        return screen.strip(content)

      content = with_retry(check_screen)

      if content == "":
        break

      press_right(url)

def device_content_lines(device: str) -> int:
  if device == "nanos":
    return 1
  if device in ["nanosp", "nanox"]:
    return 4

  raise ValueError(f"unsupported device '{device}'")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Check a section of pages contain the expected content.")
    parser.add_argument("-d", "--device", help="device type: nanos | nanosp", required=True)
    parser.add_argument("-u", "--url", help="SPECULOS_URL", required=True)
    parser.add_argument("-t", "--title", help="Section title", required=True)
    parser.add_argument("-e", "--expected-content", help="Expected content of section", required=True)
    args = parser.parse_args()

    content_lines = device_content_lines(args.device)
    content = args.expected_content

    check_multi_screen(args.url, args.title, content, content_lines, args.device)
