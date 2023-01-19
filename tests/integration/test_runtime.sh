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
        (echo "FAILURE(expect_full_text):"
         echo "  Result: '`cat $vars_dir/apdu`'"
         echo "  Expected: '$2'") >&2
        exit 1
    fi
}
