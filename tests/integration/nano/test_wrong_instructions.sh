start_speculos "$seed"
expected_home
send_apdu 8001000000
expect_apdu_return $ERR_INVALID_INS
expected_home
send_apdu 8010000000
expect_apdu_return $ERR_INVALID_INS
quit_app
