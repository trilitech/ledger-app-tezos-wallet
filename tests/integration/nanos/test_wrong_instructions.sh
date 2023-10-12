start_speculos "$seed"
expect_full_text 'ready for' 'safe signing'
send_apdu 8001000000
expect_apdu_return 6d00
send_apdu 8010000000
expect_apdu_return 6d00
press_button right
press_button right
press_button both
expect_exited
