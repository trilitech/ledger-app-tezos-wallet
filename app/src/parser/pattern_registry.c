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

#include "generated_patterns.h"

bool find_pattern(uint8_t* address, char* entrypoint, const uint8_t** pattern, uint16_t* length) {
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
