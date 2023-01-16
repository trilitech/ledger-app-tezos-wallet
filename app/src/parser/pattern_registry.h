#ifndef _PATTERN_REGISTRY
#define _PATTERN_REGISTRY    1

#include "parser_state.h"

bool find_pattern(uint8_t* address, char* entrypoint, const uint8_t** pattern, uint16_t* length);
bool find_name(uint8_t* address, const char** name);

#endif /* pattern_registry.h */
