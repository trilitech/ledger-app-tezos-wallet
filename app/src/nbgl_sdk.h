/* Patch - Ledger Secure SDK

   Copyright 2023 TriliTech <contact@trili.tech>

   With code excerpts from:
   - Ledger Secure SDK <nbgl_use_case.h>, Copyright 2023 Ledger

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#pragma once

#ifdef HAVE_NBGL
#include <nbgl_use_case.h>

void nbgl_useCaseForwardOnlyReviewNoSkip(const char *,
                                         nbgl_layoutTouchCallback_t,
                                         nbgl_navCallback_t,
                                         nbgl_choiceCallback_t);
#endif  // HAVE_NBGL
