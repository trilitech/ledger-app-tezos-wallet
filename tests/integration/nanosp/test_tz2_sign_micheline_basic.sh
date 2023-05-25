sleep 0.2
expect_full_text "Tezos Wallet" "ready for" "safe signing"
send_apdu 800f000111048000002c800006c18000000080000000
expect_apdu_return 9000
send_apdu 800f81012305020000001d0100000004434143410100000004504f504f0100000006424f5544494e
expect_full_text 'Data' '{"CACA";"POPO";"BOUDlN"}'
press_button right
expect_full_text "Accept?" "Press both buttons" "to accept."
press_button both
press_button right
press_button both
expect_exited
