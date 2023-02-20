seed="zebra`for i in $(seq 1 23) ; do echo -n ' zebra' ; done`"

function attempts {
    nb=$1
    shift
    while [ $nb -gt 0 ] && ! "$@" ; do
        nb=$((nb-1))
        sleep 0.5
    done
}

function expect_full_text {
    echo -n " - expect_full_text" ; for s in "$@" ; do echo -n " \"$s\"" ; done
    IFS= exp="$*"
    nb=100
    while true ; do
        got=`curl -s localhost:5000/events?currentscreenonly=true  2> /dev/null | jq -r '[.events[].text] | add'`
        if [ "$exp" == "$got" ] ; then
            pred_full_text="$exp"
            echo
            return 0
        fi
        # poll $nb times
        # continue if screen still displays the previous expectation (lagging redisplay)
        # or a prefix of the expectation (non atomic redisplay)
        if [ $nb -eq 1 ] || ( [ "$pred_full_text" != "$got" ] && ! [[ "$exp" =~ ^"$got".* ]] ) ; then
            echo
            (echo "FAILURE(expect_full_text):"
             echo "  On screen: '$got'"
             echo "  Previously on screen: '$pred_full_text'"
             echo "  Expected: '$exp'") >&2
            exit 1
        fi
        echo -n "."
        sleep 0.2
        nb=$((nb-1))
    done
}

function press_button {
    echo " - press_button $1"
    res=`(curl -s localhost:5000/button/$1 -d '{"action":"press-and-release"}' > /dev/null 2>&1 ; echo $?)`
    if [ "$res" != 0 ] && [ "$res" != 52 ] ; then
        echo "FAILURE(press_buttton($1)): error code $res" >&2
        exit 1
    fi
}

function send_apdu {
    echo " - apdu $1"
    (rm -f $vars_dir/apdu ;
     curl -s localhost:5000/apdu -d "{\"data\":\"$1\"}" > $vars_dir/apdu.tmp 2> /dev/null ;
     mv $vars_dir/apdu.tmp $vars_dir/apdu ) &
}

function expect_apdu_return {
    echo -n " - expect_apdu_return $1"
    attempts 100 [ -f $vars_dir/apdu ]
    if ! [ -f $vars_dir/apdu ] ; then
        echo
        echo "FAILURE(expect_apdu_return)" >&2
        exit 1
    fi
    echo
    if [ "`jq -r .data < $vars_dir/apdu`" != "$1" ] ; then
        (echo "FAILURE(expect_apdu_return):"
         echo "  Result: '`cat $vars_dir/apdu`'"
         echo "  Expected: '$1'") >&2
        exit 1
    fi
}

function send_async_apdus {
    async_apdus=`mktemp`
    echo " - will send $(($#/2)) apdus"
    (while [ "$#" -ne 0 ] ; do
         apdu="$1"
         res="$2"
         shift 2
         echo " - apdu $apdu"
         if ! curl -s localhost:5000/apdu -d "{\"data\":\"$apdu\"}" > $vars_dir/apdu 2> /dev/null ; then
             echo "FAILURE(send_async_apdus)" >&2
             exit 1
         fi
         expect_apdu_return "$res"
     done ; rm $async_apdus) &
}

function expect_async_apdus_sent {
    attempts 100 test ! -f $async_apdus
    if [ -f $async_apdus ] ; then
        (echo "FAILURE(expect_async_apdus_sent)") >&2
        exit 1
    fi
    echo " - all apdus received"
}

function cleanup {
    echo "Failure."
    kill_speculos_runner
    echo -n '== Speculos log ==' ; for i in `seq 1 $((COLUMNS-18))` ; do echo -n = ; done ; echo
    cat $vars_dir/speculog
    for i in `seq 1 $COLUMNS` ; do echo -n = ; done ; echo
    rm -rf $vars_dir
}

function expect_exited {
    echo -n " - expect_exited"
    attempts 100 exited
    echo
    if ! exited ; then
        echo "FAILURE(expect_exited)" >&2
        exit 1
    fi
}

function usage {
    echo "Usage: $0 <nanos|nanosp|nanox> <app.tgz> <test directory>" >&2
    exit 1
}

function main {
    if [ "$#" -ne 3 ] ; then
        usage
    fi

    target="$1"
    tgz="$2"
    dir="$3"

    if [ "$target" != "nanos" ] && [ "$target" != "nanosp" ] && [ "$target" != "nanox" ] ; then
        usage
    fi
    if ! [ -d "$dir" ] ; then
        usage
    fi

    for test in $dir/*.sh ; do
        vars_dir=`mktemp -d`
        start_speculos_runner
        trap cleanup EXIT
        echo "Running test $test"
        . $test
        echo "Success."
        kill_speculos_runner
        rm -rf $vars_dir
        trap - EXIT
    done
}
