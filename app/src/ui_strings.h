/** Tezos Ledger application - Dynamic UI to display a stream of pages

   Copyright 2023 TriliTech <contact@trili.tech>

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

/**
 * @brief This file implements ring buffer to store the strings to be
 * displayed on the ledger screen. The ring buffer is fixed in size and
 * implemented as fixed size C char array. The ring properties are implemented
 * logically using push, drop etc. When a new element is added after checking
 * that enough space in buffer is present, otherwise the oldest element in the
 * buffer is removed. (We can also remove the newest element if desired.)
 *
 */

#ifdef TARGET_NANOS
#define BUFF_LEN 128  /// Ring buffer length of nanos
#elif defined(HAVE_BAGL)
#define BUFF_LEN 256  /// Ring buffer length for nanos2/nanox
#else
#define BUFF_LEN 512  /// Ring buffer length for stax
#endif

/**
 * @brief This struct represents the ring buffer to store title-value pairs to
 * be displayed on the ledger device screens.
 *
 *
 */
typedef struct {
    char  buffer[BUFF_LEN];  /// Stores title-value pairs
    char *start;  /// Logical start of the buffer. This is the oldest element
                  /// in the buffer and first character of that string.
    char
        *end;  /// Logical end of the buffer. This is at the end of the newest
               /// element in the buffer, after the null terminator character.
    char *internal_end;  /// Actual end of the buffer after which no data is
                         /// present. 0 <= internal_end < BUFF_LEN
    size_t count;        /// Number of strings stored in the buffer
} tz_ui_strings_t;

/**
 * @brief Resets ring buffer and set start and end to BUFF_START
 */
void ui_strings_init(void);

/**
 * @brief Push a new string to the ring buffer.
 *          Throws error if len can not be accomodated in the empty space of
 * ring buffer. Therefore, it is important to call ui_strings_can_fit before
 * pushing the string on the buffer.
 *
 * @param str: ptr to string to copy into the buffer
 * @param len: number of of chars to copy. len <= strlen(str)
 * @param out: will be set to the start of the string in the buffer
 */
void ui_strings_push(const char *str, size_t len, char **out);
/**
 * @brief Drop the logical first string from the buffer, which is pointed by
 * 'start'.
 *
 * @param str pointer to 'start' pointer of the buffer.
 */
void ui_strings_drop(char **str);
/**
 * @brief Drop the logical last string in the ring buffer, which ends with
 * 'end' pointer.
 *
 * @param str Pointer to start pointer of last string in the buffer.
 */
void ui_strings_drop_last(char **str);
/**
 * @brief Checks if the ring buffer can fit the string of length len, without
 * deleting any existing strings.
 *
 * @param len Length of string.
 * @param can_fit result of the check, true if string can fit, false
 * otherwise.
 */
void ui_strings_can_fit(size_t len, bool *can_fit);
/**
 * @brief  Append characters from input string to the last string in the
 * buffer. Exclude the null termination character.
 *
 * @param str Input string
 * @param max maximum number of characters to append.
 * @param out pointer to the end of the resultant string in buffer (Null
 * terminator) = s->end - 1
 * @return size_t Number of chars appended.
 */
size_t ui_strings_append_last(const char *str, size_t max, char **out);
