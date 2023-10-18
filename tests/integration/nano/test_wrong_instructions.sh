start_speculos "$seed"
expect_full_text "Tezos Wallet" "ready for" "safe signing"
send_apdu 8001000000
expect_apdu_return $ERR_INVALID_INS
send_apdu 8010000000
expect_apdu_return $ERR_INVALID_INS
press_button right
press_button right
press_button both
expect_exited
