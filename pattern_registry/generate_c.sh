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

# Writes a constant string variable declaration
declare_string() {
  # Store the input string hex, variable name and maximum length in variables
  line=$1
  var_name=$2
  max_length=$3

  # Use the fold command to split the line into the specified number of character lines
  declare_strings=$(fold -w "$max_length" -s <<< "$line")

  # Check if the variable name has already been used
  if [[ " ${var_names[@]} " =~ " ${var_name} " ]]
  then
    handle_error "the variable $var_name has already been defined."
  fi

  # Check if the string has already been used
  if [[ " ${strings[@]} " =~ " XX${line}XX " ]]
  then
    # If the string has already been used, reference the first variable with that string
    first_var_name="${string_var_names["$line"]}"
    echo "const char *$var_name = $first_var_name;"
  else
    # If the string has not been used, generate a definition for it and store the variable name
    strings+=("XX$lineXX")
    string_var_names["$line"]="$var_name"

    # Add four spaces at the beginning of each line and wrap each line in double quotes
    echo "const char $var_name[] ="
    echo -n "$declare_strings" | sed -e 's/^/    "/' -e 's/$/"/'
    echo ";"
  fi

  # Add the variable name to the list of used names
  var_names+=("$var_name")
}

# Create an empty array to store the list of used variable names
var_names=()

# Create an empty array to store the list of used strings
strings=()

# Create an associative array to map strings to the first variable name they were assigned to
declare -A string_var_names

# Read the list of input file names from the command line arguments
file_names=("$@")

# Check if the file names have the expected format
for file_name in "${file_names[@]}"
do
  # Check if the file name ends with "_patterns.csv"
  if [[ ! "$file_name" =~ _patterns.csv$ ]]
  then
    handle_error "invalid file name $file_name"
  fi

  # Check if the file exists
  if [[ ! -f "$file_name" ]]
  then
    handle_error "file $file_name does not exist"
  fi
done

# Create an associative array to store the entrypoints for each contract
declare -A contract_entrypoints

# Loop over the input file names
for file_name in "${file_names[@]}"
do
  # Extract the prefix from the file name
  prefix=${file_name%_patterns.csv}

  # Read the input file line by line
  while read -r line
  do
    # Skip comments and empty lines
    if grep '^[[:space:]]*#' <<< "$line" &> /dev/null || [ "$(tr -d '[:space:]' <<< "$line")" == "" ]
    then
      continue
    fi

    # Extract the pattern and variable name from the line
    IFS=',' read -r name human _ pattern <<< "$line"

    # Trim leading and trailing spaces
    name=$(sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' <<< "$name")
    human=$(sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' <<< "$human")
    pattern=$(sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' <<< "$pattern")

    # Check that the pattern is valid / has been preprocessed
    if ! grep -E '^0x([0-9a-fA-F][0-9a-fA-F])*$' <<< "$pattern" &> /dev/null
    then
        handle_error "pattern $pattern is not in valid hexadecimal notation"
    fi

    # Record the entrypoint in the association table
    contract_entrypoints["$prefix"]="${contract_entrypoints["$prefix"]} $name"

    # Remove the "0x" prefix from the pattern
    pattern=${pattern#0x}

    # Escape each sequence of two hexadecimal digits in C style
    pattern=$(sed -e 's/\(..\)/\\x\1/g' <<< "$pattern")

    # Generate the pattern variable name as "<prefix>_<name>_pattern"
    var_name_pattern="$prefix"_"$name"_pattern

    # Generate the entrypoint variable name as "<prefix>_<name>_entrypoint"
    var_name_entrypoint="$prefix"_"$name"_entrypoint

    # Generate the display name variable name as "<prefix>_<name>_human"
    var_name_human="$prefix"_"$name"_human

    # Declare the pattern and variable name and the maximum length
    declare_string "$name" "$var_name_entrypoint" 88

    # Declare the pattern and variable name and the maximum length
    declare_string "$human" "$var_name_human" 88

    # Declare the pattern and variable name and the maximum length
    declare_string "$pattern" "$var_name_pattern" 88

    # Generate the pattern length variable name as "<prefix>_<name>_pattern_length"
    var_name_length="$prefix"_"$name"_pattern_length

    # Generate the pattern length variable declaration
    echo "#define $var_name_length $((${#pattern} / 4))"
  done < "$file_name"
done >> "$tmp_output_file"

# Create an associative array to store the type of each contract
declare -A contract_types

# Read the main input file line by line
while read -r line
do
  # Skip comments and empty lines
  if grep '^[[:space:]]*#' <<< "$line" &> /dev/null || [ "$(tr -d '[:space:]' <<< "$line")" == "" ]
  then
    continue
  fi

  # Extract the address and variable name from the line
  IFS=',' read -r name human _ address type <<< "$line"

  # Trim leading and trailing spaces
  name=$(sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' <<< "$name")
  human=$(sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' <<< "$human")
  address=$(sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' <<< "$address")
  type=$(sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' <<< "$type")

  # Check that the address is valid / has been preprocessed
  if ! grep -E '^0x([0-9a-fA-F][0-9a-fA-F])*$' <<< "$address" &> /dev/null
  then
      handle_error "address $address is not in valid hexadecimal notation"
  fi

  # Remove the "0x" prefix from the address
  address=${address#0x}

  # Escape each sequence of two hexadecimal digits in C style
  address=$(sed -e 's/\(..\)/\\x\1/g' <<< "$address")

  # Record the type of the contract
  contract_types["$name"]="$type"

  # Generate the address variable name as "contract_<name>_address"
  var_name_address="$name"_contract_address

  # Generate the human name variable name as "contract_<name>_human"
  var_name_human="$name"_contract_name

  # Declare the address and variable name and the maximum length
  declare_string "$address" "$var_name_address" 88

  # Declare the human name and variable name and the maximum length
  declare_string "$human" "$var_name_human" 88

  # Declare the contract association structure
  echo "const contract_association contract_$name = {"
  echo "  (const uint8_t*) $var_name_address, $var_name_human";
  echo "};"
done < "contracts.csv" >> "$tmp_output_file"

# Produce the main declaration
echo "const pattern_association builtin_entrypoints[] = {" >> "$tmp_output_file"

for contract in "${!contract_types[@]}"
do
  # Retrieve the contract type
  contract_type=${contract_types[$contract]}

  # Generate again the contract declaration variable name
  var_name_contract=contract_"$contract"

  # Declare the type of each entrypoint for the address
  for entrypoint in ${contract_entrypoints[$contract_type]}
  do
    # Generate again the entrypoint variable name
    var_name_entrypoint="$contract_type"_"$entrypoint"_entrypoint

    # Generate again the pattern variable name
    var_name_pattern="$contract_type"_"$entrypoint"_pattern

    # Generate again the pattern length variable name
    var_name_length="$contract_type"_"$entrypoint"_pattern_length

    echo "  { &$var_name_contract, $var_name_entrypoint, (const uint8_t*) $var_name_pattern, $var_name_length },"
  done
done >> "$tmp_output_file"
echo "  { NULL, NULL, NULL, 0 }" >> "$tmp_output_file"
echo "};" >> "$tmp_output_file"


# Dump the output buffer file
cat "$tmp_output_file"

# Remove the temporary output buffer file
rm "$tmp_output_file"
