start_speculos "$seed"
expect_full_text 'ready for' 'safe signing'
# INS_GET_PUBLIC_KEY
send_apdu 8002010011048000002c800006c18000000080000000
expect_apdu_return 6b00
send_apdu 8002800011048000002c800006c18000000080000000
expect_apdu_return 6b00
# INS_PROMPT_PUBLIC_KEY
send_apdu 8003010011048000002c800006c18000000080000000
expect_apdu_return 6b00
send_apdu 8003800011048000002c800006c18000000080000000
expect_apdu_return 6b00
press_button right
press_button right
press_button both
expect_exited