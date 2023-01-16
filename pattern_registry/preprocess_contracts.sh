#!/usr/bin/env bash

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
  IFS=',' read -r _ _ human ex_binary _ <<< "$line"

  # Trim leading and trailing spaces
  human=$(sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' <<< "$human")
  ex_binary=$(sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' <<< "$ex_binary" | tr '[:upper:]' '[:lower:]')

  # Convert human to binary
  binary="0x"$(octez-codec encode 015-PtLimaPt.contract from '"'"$human"'"')
  if [ "$?" -ne 0 ]
  then
    handle_error "error while calling octez-codec"
  fi

  # Preprocess
  if [ "$ex_binary" == "_" ]
  then
    IFS=',' read -r f0 f1 f2 _ f4 <<< "$line"
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
