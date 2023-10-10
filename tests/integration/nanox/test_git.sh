start_speculos "$seed"
sleep 0.2
expect_full_text "Tezos Wallet" "ready for" "safe signing"
send_apdu 8009000000
expect_apdu_return "$COMMIT_BYTES""00""9000"
press_button right
expect_full_text "Settings"
press_button right
expect_full_text "Quit?"
press_button both
expect_exited
