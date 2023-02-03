function expect_full_text {
    echo " - expect_full_text \"$1\""
    got=`curl -s localhost:5000/events?currentscreenonly=true  2> /dev/null | jq '[.events[].text] | add'`
    exp="$1"
    if [ " \"$exp\" " != " $got " ] ; then
        (echo "FAILURE(expect_full_text):"
         echo "  On screen: '$got'"
         echo "  Expected: '$exp'") >&2
        exit 1
    fi
}

function press_button {
    echo " - press_button $1"
    res=`(curl -s localhost:5000/button/$1 -d '{"action":"press-and-release"}' > /dev/null 2>&1 ; echo $?)`
    if [ " $res " != " 0 " ] && [ " $res " != " 52 " ] ; then
        echo "FAILURE(press_buttton($1)): error code $res" >&2
        exit 1
    fi
}

function send_apdu {
    echo " - apdu $1"
    curl -s localhost:5000/apdu -d "{\"data\":\"$1\"}" > $vars_dir/apdu 2> /dev/null &
}

function expect_apdu_return {
    echo " - expect_apdu_return $1"
    if [ "`jq .data < $vars_dir/apdu`" != "\"$1\"" ] ; then
        (echo "FAILURE(expect_apdu_return):"
         echo "  Result: '`cat $vars_dir/apdu`'"
         echo "  Expected: '$1'") >&2
        exit 1
    fi
}

function send_async_apdus {
    async_apdus=`mktemp`
    (while [ "$#" -ne 0 ] ; do
         apdu="$1"
         res="$2"
         shift 2
         echo " - apdu $apdu"
         curl -s localhost:5000/apdu -d "{\"data\":\"$apdu\"}" > $vars_dir/apdu 2> /dev/null
         expect_apdu_return "$res"
     done ; rm $async_apdus) &
}

function expect_async_apdus_sent {
    if [ -f $async_apdus ] ; then
        (echo "FAILURE(expect_async_apdus_sent)") >&2
        exit 1
    fi
}
