# Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
# Copyright 2023 Trilitech <contact@trili.tech>
# Copyright 2023 Functori <contact@functori.com>
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

COMMIT=$(git describe --tags --abbrev=8 --always --long --dirty 2>/dev/null | sed 's/-dirty/*/')
export COMMIT_BYTES=$(printf '%s' "$COMMIT" | xxd -p -c 256)

VERSION_WALLET_TAG="00"
APPVERSION_M=3
APPVERSION_N=0
APPVERSION_P=0
APPVERSION=$APPVERSION_M.$APPVERSION_N.$APPVERSION_P
export VERSION_BYTES=$(printf "%02x%02x%02x%02x" "$VERSION_WALLET_TAG" "$APPVERSION_M" "$APPVERSION_N" "$APPVERSION_P")

## Error code
ERR_WRONG_PARAM=6b00
ERR_WRONG_LENGTH=6c00
ERR_INVALID_INS=6d00
ERR_WRONG_LENGTH_FOR_INS=917e
ERR_REJECT=6985
ERR_PARSE_ERROR=9405
ERR_REFERENCED_DATA_NOT_FOUND=6a88
ERR_WRONG_VALUES=6a80
ERR_SECURITY=6982
ERR_HID_REQUIRED=6983
ERR_CLASS=6e00
ERR_MEMORY_ERROR=9200
ERR_UNEXPECTED_STATE=9001
ERR_UNEXPECTED_SIGN_STATE=9002
ERR_UNKNOWN_CX_ERR=9003
ERR_UNKNOWN=90ff
