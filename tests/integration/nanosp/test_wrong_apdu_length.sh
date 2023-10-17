start_speculos "$seed"
expect_full_text "Tezos Wallet" "ready for" "safe signing"
send_apdu 800000000000
expect_apdu_return $ERR_WRONG_LENGTH_FOR_INS
send_apdu 8000000001
expect_apdu_return $ERR_WRONG_LENGTH_FOR_INS
press_button right
press_button right
press_button both
expect_exited
