# Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
# Copyright 2023 Trilitech <contact@trili.tech>
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
#              the port $PORT.  seed (defined below) should be
#              passed to --seed on invocation to ensure that the
#              PRNG is reproducible.  Obviously headless because
#              we are testing.  And the target is in an env var
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

function attempts {
    nb=2000
    while (( nb > 0 )); do
        if "$@" ; then
            return 0
        fi
        (( nb -= 1 )) || :
        sleep 0.05
    done
    return 1
}

function get_screen_text {
   got="$(curl -s $SPECULOS_URL/events?currentscreenonly=true)"
   echo $got | jq -r '[.events[].text] | add'
}

function expect_full_text {
    echo -n " - expect_full_text" ; for s in "$@" ; do echo -n " \"$s\"" ; done
    IFS= exp="$*"
    nb=200
    FULL_TEXT_PREV=""
    while :; do
        got="$(get_screen_text)"
        if [ "$exp" == "$got" ] ; then
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
function expect_section_content {
    title="$1"
    expected_content="$2"
    echo -n " - expect_full_text_multiscreen(title: $title, content: $expected_content)"

    nb=200
    while ! echo "$(get_screen_text)" | sed '/^'"$title"'/!{q1}'; do
        sleep 0.05
        nb=$((nb-1))
        if [ $nb -eq 1 ]; then
            echo
            (echo "FAILURE(expect_section_content):"
             echo "  Title not found") >&2
            exit 1
        fi
    done

    TEXT_SO_FAR=""
    TEXT_PREV=""
    content=""
    while TEXT_PREV=$(get_screen_text) && content=$(echo "$TEXT_PREV" | sed '/^'"$title"'/!{q1}; {s/^'"$title"'\(.*\)$/\1/}');
    do
        TEXT_PREV=$content
        TEXT_SO_FAR+="$content"

        press_button right

        nb=200
        while [ "$TEXT_PREV" == "$(get_screen_text)" ]; do
            echo "waiting for screen advance"
            sleep 0.1
            nb=$((nb-1))
            if [ $nb -eq 1 ]; then
                echo
                (echo "FAILURE(expect_section_content):"
                 echo "  Screen not advancing") >&2
                exit 1
            fi
        done
    done

    press_button left

    if [ "$TEXT_SO_FAR" != "$expected_content" ]; then
        (echo "FAILURE(expect_multiscreen_content):"
         echo "  On screen: '$TEXT_SO_FAR'"
         echo "  Expected:  '$expected_content'") >&2
        exit 1
    fi
}

#
# in press_button, the ledger can return an empty reply which curl flags
# as an error (52).  This is not "no reply" and it is valid.

function press_button {
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

function send_apdu {
    echo " - apdu $1"
    APDU=$DATA_DIR/apdu-$PORT

    rm -f $APDU
    rm -f $APDU.tmp
    ( curl -s $SPECULOS_URL/apdu -d "{\"data\":\"$1\"}" > $APDU.tmp
    mv $APDU.tmp $APDU ) &
}

function expect_apdu_return {
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
}

function send_async_apdus {
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

function expect_async_apdus_sent {
    attempts test ! -f $async_apdus
    if [ -f $async_apdus ] ; then
        (echo "FAILURE(expect_async_apdus_sent)") >&2
        exit 1
    fi
    echo " - all apdus received"
}

function check_tlv_signature_from_sent_apdu {
    echo -n " - check_tlv_signature_from_apdus_sent $@"
    APDU=$DATA_DIR/apdu-$PORT
    if ! attempts [ -f $APDU ]; then
        echo "FAILURE(check_tlv_signature_from_apdus_sent)" >&2
        exit 1
    fi
    result="$(jq -r .data < $APDU)"
    $(dirname $0)/check_tlv_signature.py $@ $result
    res=$?
    set -e
    if [ "$res" != 0 ] ; then
        (echo "FAILURE(check_tlv_signature_from_apdus_sent):"
	 echo "  Check: check_tlv_signature.py $@ $result") >&2
        exit 1
    fi
}

run_a_test() {
    PORT=$1
    CMD="$2"
    pid=$BASHPID    # XXXrcd: $BASHPID is only good for bash and mksh
                    # XXXrcd: should replace with $(sh -c 'echo $PPID')
    JOBID=$RANDOM   # XXXrcd: does POSIX /bin/sh have $RANDOM?
    RETF=$(mktemp $DATA_DIR/ret-$PORT-$JOBID-XXXXXX)
    OUTF=$(mktemp $DATA_DIR/stdout-$PORT-$JOBID-XXXXXX)
    ERRF=$(mktemp $DATA_DIR/stderr-$PORT-$JOBID-XXXXXX)
    SPECULOG=$(mktemp $DATA_DIR/speculog-$PORT-$JOBID-XXXXXX)

    SPECULOS_URL=http://localhost:$PORT

    set +e
    (   set -e
        echo "PID: $$"          # get the PID into files
        echo "PORT: $PORT"
        echo "SPECULOS_URL: $SPECULOS_URL"
        if [ $TEST_TRACE = 1 ]; then
            set -x
        fi
        kill_speculos_runner
        start_speculos_runner
        . $CMD
        kill_speculos_runner
     ) > $OUTF 2> $ERRF
     RETCODE=$?
     set -e

    jq -sR --rawfile stdout $OUTF --rawfile stderr $ERRF       \
          --rawfile speculog $SPECULOG                         \
    '
        { "path":     "'"$CMD"'"
        , "retcode":  "'$RETCODE'"
        , "stdout":   $stdout
        , "stderr":   $stderr
        , "speculog": $speculog
        }
    ' > $RETF < /dev/null
    if [ -s $RETF ]; then
        # XXXrcd: we presume that these are in $RETF if it is
        #         non-zero:
        rm $OUTF $ERRF $SPECULOG
    fi
}

MAX_DOTS=40
TEST_BANNER="%-30.30s  "
TEST_BANNER_HEAD="$TEST_BANNER|                                        |\n"
test_a_dir() {
    DIR="$1"

    set -- $DIR/*.sh                # XXXrcd: maybe *.t?

    num_left=$#
    if [ -n "$TESTS_LEFT" ]; then
        num_left=$TESTS_LEFT
    fi

    DOT=0
    DID_DOT=0
    DOT_PER_NUM=$(( $num_left / $MAX_DOTS + 1 ))

    printf "$TEST_BANNER|" $DIR

    PIDS=" "
    while :; do
        for port in $(seq 5000 $((5000 + $num_left - 1)) ); do
            SLOTNAME=SLOT${port}_PID

            eval pid=\$$SLOTNAME
            if [ -z "$pid" -a -n "$1" -a "$num_left" -gt 0 ]; then
                (( num_left -= 1 )) || :
                job=$1
                shift
                if [ -n "$ONLY_TESTS"                                   \
                     -a "${ONLY_TESTS/$job/}" = "$ONLY_TESTS" ]; then
                    continue
                fi
                run_a_test $port "$job" &
                new_pid=$!
                eval $SLOTNAME=\$new_pid
                eval PIDS$new_pid=\$port
                PIDS="$PIDS$new_pid "
            fi
        done

        wait -p DIE -n $PIDS || true

        if [ -n "$DIE" ]; then
            if (( DOT >= DOT_PER_NUM )); then
                echo -n .
                DOT=0
                (( DID_DOT += 1 )) || :
            fi
            if (( DOT % 4 == 0 )); then
                echo -en '/\e[1D'
            elif (( DOT % 4 == 1)); then
                echo -en '-\e[1D'
            elif (( DOT % 4 == 2)); then
                echo -en '\\\e[1D'
            else
                echo -en '|\e[1D'
            fi
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

function cleanup {
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

function expect_exited {
    echo -n " - expect_exited"
    attempts exited
    echo
    if ! exited ; then
        echo "FAILURE(expect_exited)" >&2
        exit 1
    fi
}

function usage {
    echo "$@"                                                  >&2
    echo "Usage: $0 [-l lim] type app dir [dir ...]"           >&2
    echo "    where type is nanos, nanosp, or nanox"           >&2
    echo "     and app is a tar.gz containing the app"         >&2
    echo "     and the dirs contain the test scripts"          >&2
    exit 1
}

function main {

    TEST_TRACE=0
    while getopts T:l:x o; do
       case $o in
       T)  ONLY_TESTS="$ONLY_TESTS $OPTARG" ;;
       l)  TESTS_LEFT=$OPTARG               ;;
       x)  TEST_TRACE=1                     ;;
       \?) usage "Unknown option."          ;;
       esac
    done
    shift $((OPTIND - 1))

    [ $# -lt 3 ] && usage "At least three non-option arguments are required"

    target="$1";       shift
    tgz="$1";          shift

    if ! echo $target | grep -qE '^nano(s|sp|x)$'; then
       usage "Target \"$target\" must be nanos, nanosp, or nanox."
    fi

    [ ! -f "$tgz" ] && usage "Tarball \"$tgz\" does not exist."

    FINISHED_TESTING=
    trap cleanup EXIT

    NUM_SPECULOS=1
    DATA_DIR=$(mktemp -d /tmp/foo-XXXXXX)

    printf "$TEST_BANNER_HEAD" "Running tests"

    START=$(date +%s)
    for dir; do
       test_a_dir "$dir"
    done
    END=$(date +%s)

    jq -s . $DATA_DIR/ret-* | tee integration_tests.json |     \
       jq -r '
           def fails: map(select(.retcode != "0"));

             "Tests took " + ('$END'-'$START'|tostring) + " seconds."
           + "\nTotal Number of Tests: " + (.|length|tostring)
           + "\nNumber of Failures:    " + (fails|length|tostring)
           + if ((fails|length) > 0) then
                 "\nFailed cases:          "
               + "\n\t"
               + ([(fails[]|.path)]|[limit(5;.[])]|join("\n\t"))
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
