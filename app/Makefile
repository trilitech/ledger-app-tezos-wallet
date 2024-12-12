# ****************************************************************************
#    Ledger App Boilerplate
#    (c) 2023 Ledger SAS.
#    (c) 2023 Nomadic Labs <contact@nomadic.labs>
#    (c) 2023 Trilitech <contact@trili.tech>
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# ****************************************************************************

ifeq ($(BOLOS_SDK),)
$(error Environment variable BOLOS_SDK is not set)
endif

include $(BOLOS_SDK)/Makefile.defines

# Clean up on target switches, to allow rebuilding to other devices more easily
ifeq ($(file <.target),)
$(file >.target,$(TARGET_NAME)$(DEBUG))
else
ifneq ($(TARGET_NAME)$(DEBUG), $(file <.target))
$(info Target switch detected, $(file <.target) -> $(TARGET_NAME)$(DEBUG), cleaning up...)
$(file >.target,$(TARGET_NAME)$(DEBUG))
IGNORE_OUTPUT:=$(shell make clean DEBUG=$(DEBUG))
endif
endif

########################################
#        Mandatory configuration       #
########################################
# Application name
APPNAME = "Tezos Wallet"

# Application version
APPVERSION_M=3
APPVERSION_N=0
APPVERSION_P=6
APPVERSION=$(APPVERSION_M).$(APPVERSION_N).$(APPVERSION_P)

# COMMIT
GIT_DESCRIBE ?= $(shell git describe --tags --abbrev=8 --always --long --dirty 2>/dev/null)

VERSION_TAG ?= $(shell echo "$(GIT_DESCRIBE)" | cut -f1 -d-)

# Only warn about version tags if specified/inferred
ifeq ($(VERSION_TAG),)
  $(warning VERSION_TAG not checked)
else
  ifneq (v$(APPVERSION), $(VERSION_TAG))
    $(warning Version-Tag Mismatch: v$(APPVERSION) version and $(VERSION_TAG) tag)
  endif
endif

COMMIT ?= $(shell echo "$(GIT_DESCRIBE)" | awk -F'-g' '{print $2}' | sed 's/-dirty/*/')
ifeq ($(COMMIT),)
  $(warning COMMIT not specified and could not be determined with git from "$(GIT_DESCRIBE)")
else
  $(info COMMIT=$(COMMIT))
endif
DEFINES += COMMIT=\"$(COMMIT)\"

# Application source files
APP_SOURCE_PATH += src src/parser

# Application icons following guidelines:
# https://developers.ledger.com/docs/embedded-app/design-requirements/#device-icon
ICON_NANOS = icons/nano-s-tezos.gif
ICON_NANOX = icons/nano-x-tezos.gif
ICON_NANOSP = icons/nano-x-tezos.gif
ICON_STAX = icons/stax_tezos.gif
ICON_FLEX = icons/flex_tezos.gif

# Application allowed derivation curves.
# Possibles curves are: secp256k1, secp256r1, ed25519 and bls12381g1
# If your app needs it, you can specify multiple curves by using:
# `CURVE_APP_LOAD_PARAMS = <curve1> <curve2>`
CURVE_APP_LOAD_PARAMS = ed25519 secp256k1 secp256r1

# Application allowed derivation paths.
# You should request a specific path for your app.
# This serve as an isolation mechanism.
# Most application will have to request a path according to the BIP-0044
# and SLIP-0044 standards.
# If your app needs it, you can specify multiple path by using:
# `PATH_APP_LOAD_PARAMS = "44'/1'" "45'/1'"`
PATH_APP_LOAD_PARAMS = "44'/1729'"

# Setting to allow building variant applications
# - <VARIANT_PARAM> is the name of the parameter which should be set
#   to specify the variant that should be build.
# - <VARIANT_VALUES> a list of variant that can be build using this app code.
#   * It must at least contains one value.
#   * Values can be the app ticker or anything else but should be unique.
VARIANT_PARAM = APP_TYPE
VARIANT_VALUES = tezos_wallet

# Enabling DEBUG flag will enable PRINTF and disable optimizations
#DEBUG = 1
ifneq ($(DEBUG), 0)
  DEFINES += TEZOS_DEBUG
endif

# CFLAGS
ENABLE_SDK_WERROR=1
CFLAGS   += -O3 -Os -Wall -Wextra
CFLAGS   += -Wassign-enum
#CFLAGS   += -Wbad-function-cast
#CFLAGS   += -Wcast-align
#CFLAGS   += -Wcast-qual
CFLAGS   += -Wcomma
CFLAGS   += -Wdeprecated
#CFLAGS   += -Wmissing-prototypes
CFLAGS   += -Wnull-pointer-arithmetic
CFLAGS   += -Wover-aligned
#CFLAGS   += -Wpointer-arith
#CFLAGS   += -Wreserved-id-macro
CFLAGS   += -Wshadow-all
CFLAGS   += -Wsign-compare
CFLAGS   += -Wstring-compare
CFLAGS   += -Wstring-conversion
CFLAGS   += -Wuninitialized
#CFLAGS   += -Wunreachable-code-aggressive
CFLAGS   += -Wwrite-strings

########################################
#     Application custom permissions   #
########################################
# See SDK `include/appflags.h` for the purpose of each permission
#HAVE_APPLICATION_FLAG_DERIVE_MASTER = 1
HAVE_APPLICATION_FLAG_LIBRARY = 1

########################################
# Application communication interfaces #
########################################
ENABLE_BLUETOOTH = 1
#ENABLE_NFC = 1
ENABLE_SWAP = 1

# U2F - used e.g. by Kukai wallet
SDK_SOURCE_PATH  += lib_u2f lib_stusb_impl
DEFINES   += USB_SEGMENT_SIZE=64
DEFINES   += U2F_PROXY_MAGIC=\"XTZ\"
DEFINES   += HAVE_IO_U2F HAVE_U2F

########################################
#         NBGL custom features         #
########################################
ENABLE_NBGL_QRCODE = 1
#ENABLE_NBGL_KEYBOARD = 1
#ENABLE_NBGL_KEYPAD = 1

########################################
#          Features disablers          #
########################################
# These advanced settings allow to disable some feature that are by
# default enabled in the SDK `Makefile.standard_app`.
#DISABLE_STANDARD_APP_FILES = 1
#DISABLE_DEFAULT_IO_SEPROXY_BUFFER_SIZE = 1 # To allow custom size declaration
#DISABLE_STANDARD_APP_DEFINES = 1 # Will set all the following disablers
#DISABLE_STANDARD_SNPRINTF = 1
#DISABLE_STANDARD_USB = 1
#DISABLE_STANDARD_WEBUSB = 1
#DISABLE_STANDARD_BAGL_UX_FLOW = 1

include $(BOLOS_SDK)/Makefile.standard_app

.PHONY: mrproper
mrproper: clean
	rm -f .target
