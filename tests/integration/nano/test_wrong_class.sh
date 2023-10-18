start_speculos "$seed"
expected_home
send_apdu 0000000000
expect_apdu_return $ERR_CLASS
expected_home
send_apdu 8100000000
expect_apdu_return $ERR_CLASS
quit_app
