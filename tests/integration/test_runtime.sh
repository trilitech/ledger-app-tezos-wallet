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
#
# This library defines a number of functions that will be used to
# provide an environment to test using a few instances of speculos.
#
# The library sets a number of global variables:
#
#      DATA_DIR        this dir is where all state goes.  This
#                      state is global.
#      PORT            the port on which speculos listens.  This
#                      will vary as we run multiple instances.
#      SPECULOS_URL    the URL for your process's speculos instance
#      SPECULOG        tmp file for this process's speculos log output
#
# To use this library, you must provide the following functions:
#
#      start_speculos_runner
#              This function will start a speculos runner on
#              the port $PORT.  The seed passed in argument should be
#              passed to --seed on invocation to ensure that the
#              PRNG is reproducible.  Obviously headless because
#              we are testing.  And the TARGET is in an env var
#              of the same name.  Logs must be stored in $SPECULOG.
#
#      kill_speculos_runner
#
#              kill the thing.  Must wait for it.
#
#      exited
#
#              True if it's gone.
#

seed="zebra`for i in $(seq 1 23) ; do echo -n ' zebra' ; done`"
OUTPUT_BARS=$(for i in $(seq 1 $((COLUMNS-18))); do echo -n =; done)

attempts() {
    nb=$(( $TIMEOUT * 20 ))    # We use multiplication to avoid floats
    while (( nb > 0 )); do
        if "$@" ; then
            return 0
        fi
        (( nb -= 1 )) || :
        sleep 0.05
    done
    return 1
}

compare_strings() {
    STR1="$1"
    STR2="$2"

    if [ "nanox" = "$TARGET" ]; then
        # TODO: raise issue on speculos?
        STR1="$(echo $1 | sed 's/Parsing errorERR//g')"
        STR2="$(echo $2 | sed 's/Parsing errorERR//g')"
    fi

    [ "$STR1" = "$STR2" ]
}

get_screen_text() {
   got="$(curl -s $SPECULOS_URL/events?currentscreenonly=true)"
   echo $got | jq -r '[.events[].text] | add'
}

expect_full_text() {
    echo -n " - expect_full_text" ; for s in "$@" ; do echo -n " \"$s\"" ; done
    IFS= exp="$*"
    nb=200
    FULL_TEXT_PREV=""
    while :; do
        got="$(get_screen_text)"
        if compare_strings "$exp" "$got"; then
            FULL_TEXT_PREV="$exp"
            echo
            return 0
        fi
        # poll $nb times
        # continue if screen still displays the previous expectation
        # (lagging redisplay) or a prefix of the expectation (non
        # atomic redisplay)
        if [ $nb -eq 1 ]; then
            echo
            (echo "FAILURE(expect_full_text):"
             echo "  On screen: '$got'"
             echo "  Previously on screen: '$FULL_TEXT_PREV'"
             echo "  Expected: '$exp'") >&2
            exit 1
        fi
        echo -n "."
        sleep 0.05
        nb=$((nb-1))
    done
}

# One section of data can spill across multiple screens.
# Collect all pages with the given title, and then compare at the end.
expect_section_content() {
    echo -n " - expect_section_content $1"

    $(dirname $0)/check_section_text.py --device=$TARGET --url=$SPECULOS_URL --title="$1" --expected-content="$2"

    res=$?
    set -e
    if [ "$res" != 0 ] ; then
        (echo "FAILURE(expect_section_content):"
         echo "  Check: expect_section_content.py $@ $result") >&2
        exit 1
    fi
}

#
# in press_button, the ledger can return an empty reply which curl flags
# as an error (52).  This is not "no reply" and it is valid.

press_button() {
    echo " - press_button $1"
    set +e
    curl -s $SPECULOS_URL/button/$1 -d '{"action":"press-and-release"}' \
       >/dev/null 2>&1
    res=$?
    set -e
    if [ "$res" != 0 ] && [ "$res" != 52 ] ; then
        echo "FAILURE(press_buttton($1)): error code $res" >&2
        exit 1
    fi
}

expected_home() {
    echo " - expected_home"
    if [ "$TARGET" == "nanos" ]; then
	expect_full_text 'ready for' 'safe signing'
    else
	expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
    fi
}

expected_blind_home() {
    echo " - expected_blind_home"
    if [ "$TARGET" == "nanos" ]; then
	expect_full_text 'ready for' 'BLIND signing'
    else
	expect_full_text 'Tezos Wallet' 'ready for' 'BLIND signing'
    fi
}

expected_version() {
    echo " - expected_version"
    expect_full_text 'Version' $APPVERSION
}

expected_verify_address() {
    echo " - expected_verify_address"
    if [ "$TARGET" == "nanos" ]; then
	expect_full_text 'Verify address'
    else
	expect_full_text 'Verify' 'address'
    fi
}

set_blind_signing() {
    echo " - set_blind_signing"
    expected_home
    press_button right
    expected_version
    press_button right
    expect_full_text "Settings"
    press_button both
    expect_full_text 'Blind Signing' 'DISABLED'
    press_button both
    expect_full_text 'Blind Signing' 'ENABLED'
    press_button right
    expect_full_text 'Back'
    press_button both
    expected_home
}

quit_app() {
    echo " - quit_app"
    expected_home
    press_button right
    expected_version
    press_button right
    expect_full_text "Settings"
    press_button right
    expect_full_text "Quit?"
    press_button both
    expect_exited
}

quit_blind_app() {
    echo " - quit_blind_app"
    expected_home
    press_button right
    expected_blind_home
    press_button right
    expected_version
    press_button right
    expect_full_text "Settings"
    press_button right
    expect_full_text "Quit?"
    press_button both
    expect_exited
}

expected_accept() {
    echo " - expected_accept"
    if [ "$TARGET" == "nanos" ]; then
	expect_full_text 'Accept and send'
    else
	expect_full_text 'Accept' 'and send'
    fi
}

expected_accept_public_key() {
    echo " - expected_accept_public_key"
    expect_full_text 'Approve'
}

expected_reject() {
    echo " - expected_reject"
    expect_full_text 'Reject'
}

expected_parsing_error() {
    echo " - expected_parsing_error $1"
    if [ "$TARGET" == "nanos" ]; then
	expect_full_text 'Parsing error'
    else
	expect_full_text 'Parsing error' $1
    fi
}

send_apdu() {
    echo " - apdu $1"
    APDU=$DATA_DIR/apdu-$PORT

    if [ "$APDU_OUTSTANDING" = 1 ]; then
        echo "ERROR IN TEST, ERROR IN TEST"                        >&2
        echo "send_apdu() called without expecting a return..."    >&2
        exit 1
    fi

    APDU_OUTSTANDING=1

    rm -f $APDU
    rm -f $APDU.tmp
    ( curl -s $SPECULOS_URL/apdu -d "{\"data\":\"$1\"}" > $APDU.tmp
    mv $APDU.tmp $APDU ) &
}

expect_apdu_return() {
    echo -n " - expect_apdu_return $1"
    APDU=$DATA_DIR/apdu-$PORT
    if ! attempts [ -f $APDU ]; then
        echo "FAILURE(expect_apdu_return)" >&2
        exit 1
    fi
    echo
    result="$(jq -r .data < $APDU)"
    if [ "$result" != "$1" ]; then
        (echo "FAILURE(expect_apdu_return):"
         echo "  Result: '$result'"
         echo "  Expected: '$1'") >&2
        exit 1
    fi
    APDU_OUTSTANDING=0
}

send_async_apdus() {
    APDU=$DATA_DIR/apdu-$PORT
    async_apdus="$(mktemp $DATA_DIR/async_apdus-XXXXXX)"
    echo " - will send $(($#/2)) apdus"
    (while [ "$#" -ne 0 ] ; do
         apdu="$1"
         check="$2"
         shift 2
         echo " - apdu $apdu"
         if ! curl -s $SPECULOS_URL/apdu -d "{\"data\":\"$apdu\"}" \
                                         > $APDU 2> /dev/null ; then
             echo "FAILURE(send_async_apdus)" >&2
             exit 1
         fi
         eval "$check"
     done ; rm $async_apdus) &
}

expect_async_apdus_sent() {
    if ! attempts test ! -f $async_apdus; then
        echo "FAILURE(expect_async_apdus_sent)" >&2
        exit 1
    fi
    echo " - all apdus received"
}

check_tlv_signature() {
    echo -n " - check_tlv_signature $@"
    APDU=$DATA_DIR/apdu-$PORT
    if ! attempts [ -f $APDU ]; then
        echo "FAILURE(check_tlv_signature)" >&2
        exit 1
    fi
    result="$(jq -r .data < $APDU)"
    $(dirname $0)/check_tlv_signature.py $@ $result
    res=$?
    set -e
    if [ "$res" != 0 ] ; then
        (echo "FAILURE(check_tlv_signature):"
         echo "  Check: check_tlv_signature.py $@ $result") >&2
        exit 1
    fi
}

until_failure() {
    echo " - until_failure $@"
    (eval "$@")
    res=$?
    if [ "$res" == 0 ] ; then
        (echo "FAILURE(until_failure)") >&2
        exit 1
    fi
    echo " - until_failure succeeded"
}

start_speculos() {
    start_speculos_runner $DBG "$1"
    set -e
    trap kill_speculos_runner EXIT
}

run_a_test() {
    DBG=$1
    PORT=$2
    CMD="$3"
    pid=$BASHPID    # XXXrcd: $BASHPID is only good for bash and mksh
                    # XXXrcd: should replace with $(sh -c 'echo $PPID')
    JOBID=$RANDOM   # XXXrcd: does POSIX /bin/sh have $RANDOM?
    RETF=$(mktemp $DATA_DIR/ret-$PORT-$JOBID-XXXXXX)
    OUTF=$(mktemp $DATA_DIR/stdout-$PORT-$JOBID-XXXXXX)
    ERRF=$(mktemp $DATA_DIR/stderr-$PORT-$JOBID-XXXXXX)
    SPECULOG=$(mktemp $DATA_DIR/speculog-$PORT-$JOBID-XXXXXX)

    SPECULOS_URL=http://localhost:$PORT
    TIMESTAMP=$(date +%s)

    set +e
    (   echo "PID: $$"          # get the PID into files
        echo "PORT: $PORT"
        echo "SPECULOS_URL: $SPECULOS_URL"
        if [ $TEST_TRACE = 1 ]; then
            set -x
        fi
        (
            case $CMD in
                *.sh)
                    . $CMD
                    ;;
                *.py)
                    start_speculos "$seed"
                    PORT=$PORT\
                        COMMIT_BYTES=$COMMIT_BYTES\
                        VERSION_BYTES=$VERSION_BYTES\
                        python3 $CMD
                    ;;
                *.hex)
                    # We skip these...
                    ;;
                *)
                    if [ -f "$CMD" ]; then
                        echo Command "$CMD" ends in neither .sh nor .py >&2
                        exit 1;
                    fi
                    ;;
            esac
        )
    ) > $OUTF 2> $ERRF
    RETCODE=$?
    set -e

    if [ $RETCODE = 0 -a "$ONLY_FAILURES" = "YES" ]; then
        rm $OUTF $ERRF $SPECULOG
        return $RETCODE
    fi

    jq -sR --rawfile stdout $OUTF --rawfile stderr $ERRF       \
           --rawfile script $CMD --rawfile speculog $SPECULOG  \
    '
        { "path":     "'"$CMD"'"
        , "pid":      "'$PID'"
        , "port":     "'$PORT'"
        , "retcode":  "'$RETCODE'"
        , "script":   $script
        , "stdout":   $stdout
        , "stderr":   $stderr
        , "speculog": $speculog
        , "timestamp":  "'$TIMESTAMP'"
        }
    ' > $RETF < /dev/null
    if [ -s $RETF ]; then
        # XXXrcd: we presume that these are in $RETF if it is
        #         non-zero:
        rm $OUTF $ERRF $SPECULOG
    fi
    return $RETCODE
}

run_both_tests() {
    PORT=$1
    CMD="$2"

    run_a_test NORMAL $PORT "$CMD" || run_a_test DEBUG $PORT "$CMD"
}

MAX_DOTS=40
BANSIZE=30
TEST_BANNER="%-${BANSIZE}.${BANSIZE}s  "
TEST_BANNER_HEAD="$TEST_BANNER|                                        |\n"
test_a_path() {
    THE_PATH="$1"

    if [ -d "$THE_PATH" ]; then
        set -- $THE_PATH/*
    else
        set -- $THE_PATH
    fi

    num_left=$#
    if [ -n "$TESTS_LEFT" ]; then
        num_left=$TESTS_LEFT
    fi

    DOT=0
    DID_DOT=0
    DOT_PER_NUM=$(( $num_left / $MAX_DOTS + 1 ))
    THE_DOT=.

    printf "$TEST_BANNER|" $(echo $THE_PATH | sed -E "s/.*(.{$BANSIZE})$/\1/")

    PIDS=" "
    while :; do
        for port in $(seq 5000 $((5000 + NUM_SPECULOS - 1)) ); do
            SLOTNAME=SLOT${port}_PID

            eval pid=\$$SLOTNAME
            if [ -z "$pid" -a -n "$1" -a "$num_left" -gt 0 ]; then
                (( num_left -= 1 )) || :
                job=$1
                shift
                run_both_tests $port "$job" &
                new_pid=$!
                eval $SLOTNAME=\$new_pid
                eval PIDS$new_pid=\$port
                PIDS="$PIDS$new_pid "
            fi
        done

        wait -p DIE -n $PIDS || THE_DOT=\*

        if [ -n "$DIE" ]; then
            if (( DOT >= DOT_PER_NUM )); then
                echo -n "$THE_DOT"
                THE_DOT=.
                DOT=0
                (( DID_DOT += 1 )) || :
            fi
            (( TESTS_RUN += 1 )) || :
            (( DOT += 1 )) || :
            PIDS="${PIDS/ $DIE / }"
            eval slot=\$PIDS$DIE
            eval SLOT${slot}_PID=
        fi

        if [ \( -z "$1" -o "$num_left" -eq 0 \) \
             -a "${PIDS/[0-9]*/}" = "$PIDS" ]; then
            # XXXrcd: we are done.
            break
        fi
    done
    while (( $DID_DOT < $MAX_DOTS )); do
        echo -n .
        (( DID_DOT += 1 )) || :
    done
    echo "|"
    if [ -n "$TESTS_LEFT" ]; then
        TESTS_LEFT="$num_left"
    fi
}

cleanup() {
    retcode=$?

    if [ -z "$FINISHED_TESTING" -a "$retcode" != 0 ]; then
        # XXXrcd: BROKEN!
        kill_speculos_runner
        echo $OUTPUT_BARS
        echo Something went wrong in the test framework.
        if [ -d "$DATA_DIR" ]; then
            echo The files in "$DATA_DIR" might be able
            echo to help troubleshooting.
        fi
        echo $OUTPUT_BARS
    else
        rm -rf $DATA_DIR
    fi
}

expect_exited() {
    echo -n " - expect_exited"
    attempts exited
    echo
    if ! exited ; then
        echo "FAILURE(expect_exited)" >&2
        exit 1
    fi
}

usage() {
    echo "$@"                                                            >&2
    echo -n "Usage: $0 [-F] [-l lim] [-m arch] [-t tgz] [-d tgz] "       >&2
    echo               "path [path...]"                                  >&2
    echo "    where paths are either a test or a dir containing tests"   >&2
    echo "            -F means that only failures are stored"            >&2
    echo "            -d tgz specifies that tgz contains the debug app"  >&2
    echo "            -l lim limits the number of tests run to lim"      >&2
    echo "            -m arch is one of nanos, nanosp, nanox, or stax"   >&2
    echo "            -t tgz specifies that tgz contains the app"        >&2
    exit 1
}

main() {

    # Defaults:
    TARGET=nanos
    TEST_TRACE=0
    export TIMEOUT=20
    NUM_SPECULOS=32

    while getopts FT:l:m:n:t:x o; do
       case $o in
       F)  ONLY_FAILURES=YES                ;;
       T)  TIMEOUT="$OPTARG"                ;;
       d)  DTGZ="$OPTARG"                   ;;
       l)  TESTS_LEFT=$OPTARG               ;;
       m)  TARGET="$OPTARG"                 ;;
       n)  NUM_SPECULOS="$OPTARG"           ;;
       t)  TGZ="$OPTARG"                    ;;
       x)  TEST_TRACE=1                     ;;
       \?) usage "Unknown option."          ;;
       esac
    done
    shift $((OPTIND - 1))

    [ $# -lt 1 ] && usage "At least one test must be provided"

    if ! echo $TARGET | grep -qE '^(stax)|(nano(s|sp|x))$'; then
       usage "Target \"$TARGET\" must be nanos, nanosp, nanox or stax."
    fi

    if [ -z "$TGZ" ]; then
        TGZ="./app_${TARGET}.tgz"
    fi

    if [ -z "$DTGZ" ]; then
        DTGZ="./app_${TARGET}_dbg.tgz"
    fi

    [ ! -f "$TGZ" ]  && usage "Tarball \"$TGZ\" does not exist."
    [ ! -f "$DTGZ" ] && usage "Debug Tarball \"$DTGZ\" does not exist."

    FINISHED_TESTING=
    TESTS_RUN=0
    trap cleanup EXIT

    DATA_DIR=$(mktemp -d /tmp/foo-XXXXXX)

    printf "$TEST_BANNER_HEAD" "Running tests"

    START=$(date +%s)
    for dir; do
       test_a_path "$dir"
    done
    END=$(date +%s)

    jq -s . $DATA_DIR/ret-* | tee integration_tests.json |     \
       jq -r '
           def num_tests: [.[].path]|unique|length|tostring;
           def fails: map(select(.retcode != "0"));

             "Tests took " + ('$END'-'$START'|tostring) + " seconds."
           + "\nTotal Number of Tests: " + ('$TESTS_RUN'|tostring)
           + "\nNumber of Failures:    " + (fails|num_tests)
           + if ((fails|length) > 0) then
                 "\nFailed cases:          "
               + "\n\t"
               + ([(fails[]|.path)]|unique|[limit(5;.[])]|join("\n\t"))
               + if ((fails|length) > 5) then "\n\t..." else "" end
               + "\n\nFailures occurred in the test suite.  Please find"
               + "\nthe results in ./integration_tests.json"
             else "" end
           + "\n"
       '

    FINISHED_TESTING=1
    < integration_tests.json >/dev/null \
        jq -e 'map(select(.retcode != "0"))|length == 0'
}
