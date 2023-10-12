start_speculos "$seed"
expect_full_text 'ready for' 'safe signing'
send_apdu 800000000000
expect_apdu_return 917e
send_apdu 8000000001
expect_apdu_return 917e
press_button right
press_button right
press_button both
expect_exited
