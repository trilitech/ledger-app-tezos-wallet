start_speculos "$seed"
sleep 0.2
expected_home
send_apdu 8003000011048000002c800006c18000000080000000
expect_section_content "Provide Key" 'tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E'
press_button right
expected_accept
press_button right
expected_reject
press_button both
expect_apdu_return $ERR_REJECT
quit_app
