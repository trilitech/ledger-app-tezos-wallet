start_speculos "$seed"
sleep 0.2
expected_home
send_apdu 8003000111048000002c800006c18000000080000000
expect_section_content "Provide Key" 'tz2GB5YHqF4UzQ8GP5yUqdhY9oVWRXCY2hPU'
press_button right
expected_accept
press_button both
expect_apdu_return 4104211f369d9ec3a0fbe10febf05a8b67a9ca705b90534b9dfe3e08f2dac99bd008111079d35265a935866e0444f1a4c1044c9c81f4178d43ef5c132332019bf9c19000
quit_app
