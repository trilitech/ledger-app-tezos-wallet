#!/usr/bin/env bash

# Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Use a temporary output buffer file to prevent generating a partial output in case of an error
tmp_output_file=$(mktemp)

# Handle an error by displaying an error message, removing the output buffer and exiting
handle_error() {
  # Display the error message
  echo "Error: $1" >&2

  # Remove the output buffer file
  rm "$tmp_output_file"

  # Exit with a status code of 1
  exit 1
}

# Check if argument present
if [[ "$#" -lt 1 ]]
then
  handle_error "expecting a file as argument"
fi

# Check if the file exists
if [[ ! -f "$1" ]]
then
  handle_error "file $1 does not exist"
fi

# Read the main input file line by line
while read -r line
do
  # Skip comments and empty lines
  if grep '^[[:space:]]*#' <<< "$line" &> /dev/null || [ "$(tr -d '[:space:]' <<< "$line")" == "" ]
  then
    echo "$line"

    continue
  fi

  # Extract the address and variable name from the line
  IFS=',' read -r pat _ human ex_binary <<< "$line"

  # Trim leading and trailing spaces
  human=$(sed -e 's/\\\n//g' -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' <<< "$human")
  ex_binary=$(tr -d '\n\\' <<< "$ex_binary" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' | tr '[:upper:]' '[:lower:]')

  # Convert human to binary
  binary="0x"$(dune exec ./encode_pattern.exe "$human" 2> /dev/null)
  if [ "$?" -ne 0 ]
  then
    dune exec ./encode_pattern.exe "$human"
    handle_error "in pattern $pat while calling ./encode_pattern.ml on $human"
  fi

  # Preprocess
  if [ "$ex_binary" == "_" ]
  then
    IFS=',' read -r f0 f1 f2 _ <<< "$line"
    echo "  $f0,$f1,$f2, $binary,$f4"
  else
    # If already proprocessed, just check
    if [ "$ex_binary" != "$binary" ]
    then
      handle_error "Invalid binary $ex_binary for $human in preprocessed file, should be $binary"
    else
      echo "  $line"
    fi
  fi
done < "$1" >> "$tmp_output_file"

# Dump the output buffer file
cat "$tmp_output_file"

# Remove the temporary output buffer file
rm "$tmp_output_file"
