start_speculos "$seed"
sleep 0.2
expect_full_text "ready for" "safe signing"
send_apdu 8000000000
expect_apdu_return "$VERSION_BYTES""9000"
press_button right
expect_full_text "Settings"
press_button right
expect_full_text "Quit?"
press_button both
expect_exited
