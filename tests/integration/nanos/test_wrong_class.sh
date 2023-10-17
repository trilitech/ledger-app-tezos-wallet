start_speculos "$seed"
expect_full_text 'ready for' 'safe signing'
send_apdu 0000000000
expect_apdu_return $ERR_CLASS
send_apdu 8100000000
expect_apdu_return $ERR_CLASS
press_button right
press_button right
press_button both
expect_exited
