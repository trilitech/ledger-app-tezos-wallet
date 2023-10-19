start_speculos "$seed"
sleep 0.2
# INS_AUTHORIZE_BAKING
expected_home
send_apdu 8001000000
expect_apdu_return $ERR_INVALID_INS
# INS_SIGN_UNSAFE
expected_home
send_apdu 8005000000
expect_apdu_return $ERR_INVALID_INS
# INS_RESET
expected_home
send_apdu 8006000000
expect_apdu_return $ERR_INVALID_INS
# INS_QUERY_AUTH_KEY
expected_home
send_apdu 8007000000
expect_apdu_return $ERR_INVALID_INS
# INS_QUERY_MAIN_HWM
expected_home
send_apdu 8008000000
expect_apdu_return $ERR_INVALID_INS
# INS_SETUP
expected_home
send_apdu 800a000000
expect_apdu_return $ERR_INVALID_INS
# INS_QUERY_ALL_HWM
expected_home
send_apdu 800b000000
expect_apdu_return $ERR_INVALID_INS
# INS_DEAUTHORIZE
expected_home
send_apdu 800c000000
expect_apdu_return $ERR_INVALID_INS
# INS_QUERY_AUTH_KEY_WITH_CURVE
expected_home
send_apdu 800d000000
expect_apdu_return $ERR_INVALID_INS
# INS_HMAC
expected_home
send_apdu 800e000000
expect_apdu_return $ERR_INVALID_INS
# Unknown instruction
expected_home
send_apdu 80ff000000
expect_apdu_return $ERR_INVALID_INS
quit_app
