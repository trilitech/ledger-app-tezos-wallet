/* Tezos Embedded C parser for Ledger - Registry of contract names and entrypoint patterns

   Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include "pattern_registry.h"

typedef struct {
  const uint8_t * address;
  const char *name;
} contract_association;

typedef struct {
  const contract_association *contract;
  const char* entrypoint;
  const uint8_t* pattern;
  const uint16_t length;
} pattern_association;

const pattern_association builtin_entrypoints[] = {
  { NULL, NULL, NULL, 0 }
};

bool find_pattern(uint8_t* address, const char* entrypoint, const uint8_t** pattern, uint16_t* length) {
  const pattern_association *assoc = &builtin_entrypoints[0];
  while(assoc->contract) {
    contract_association* contract = PIC(assoc->contract);
    if (memcmp (PIC(contract->address), address, 22) == 0 && strcmp (PIC(assoc->entrypoint), entrypoint) == 0) {
      *pattern = PIC(assoc->pattern);
      *length = assoc->length;
      return true;
    }
    assoc++;
  }
  return false;
}

bool find_name(uint8_t* address, const char** name) {
  const pattern_association *assoc = &builtin_entrypoints[0];
  while(assoc->contract) {
    contract_association* contract = PIC(assoc->contract);
    if (memcmp (PIC(contract->address), address, 22) == 0) {
      *name = PIC(contract->name);
      return true;
    }
    assoc++;
  }
  return false;
}
