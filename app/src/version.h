/* Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

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

#ifdef BAKING_APP
#define CLASS 1
#else
#define CLASS 0
#endif

#define MAJOR APPVERSION_M
#define MINOR APPVERSION_N
#define PATCH APPVERSION_P

typedef struct version {
    uint8_t class;
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} version_t;

const version_t version = {CLASS, MAJOR, MINOR, PATCH};
