#!/usr/bin/env python3
# Copyright 2023 Functori <contact@functori.com>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import base58
from pytezos import pytezos

def signature_of_tlv(tlv):
    # See: https://developers.ledger.com/docs/embedded-app/crypto-api/lcx__ecdsa_8h/#cx_ecdsa_sign
    # TLV: 30 || L || 02 || Lr || r || 02 || Ls || s
    header_tag_index = 0
    if tlv[header_tag_index] != 0x30:
        raise ValueError("Invalid TLV tag")
    len_index = 1
    if tlv[len_index] != len(tlv) - 2:
        raise ValueError("Invalid TLV length")
    first_tag_index = 2
    if tlv[first_tag_index] != 0x02:
        raise ValueError("Invalid TLV tag")
    r_len_index = 3
    r_index = 4
    r_len = tlv[r_len_index]
    second_tag_index = r_index + r_len
    if tlv[second_tag_index] != 0x02:
        raise ValueError("Invalid TLV tag")
    s_len_index = second_tag_index + 1
    s_index = s_len_index + 1
    s_len = tlv[s_len_index]
    return tlv[r_index : r_index + r_len] + tlv[s_index : s_index + s_len]

def signature_of_hex_tlv(hex_string):
    tlv = bytearray.fromhex(hex_string)
    # Remove the unwanted parity information set here.
    tlv[0] &= ~0x01
    return signature_of_tlv(tlv)

def check_between(prefix, suffix, v):
    if v.startswith(prefix) and v.endswith(suffix):
        return v[len(prefix) : -len(suffix)]
    else:
        raise ValueError("Invalid prefix/suffix")

if __name__ == "__main__":
    if len(sys.argv) != 6:
        print("Usage: check_tlv_signature.py <prefix> <suffix> <pk> <message> <hex_string>")
        sys.exit(1)

    prefix = sys.argv[1]
    suffix = sys.argv[2]
    pk = sys.argv[3]
    message = sys.argv[4]
    hex_string = sys.argv[5]

    hex_tlv = check_between(prefix, suffix, hex_string)

    sig = signature_of_hex_tlv(hex_tlv)

    sig_prefix = bytes.fromhex("04822b") # sig(96)
    signature = base58.b58encode_check(sig_prefix + sig)

    pytezos = pytezos.using(key=pk)
    assert pytezos.key.verify(signature, message)
