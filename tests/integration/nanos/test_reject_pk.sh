start_speculos "$seed"
sleep 0.2
expect_full_text "ready for" "safe signing"
send_apdu 8003000011048000002c800006c18000000080000000
expect_section_content nanos "Provide Key" \
                             'tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E'
press_button right
expect_full_text "Accept?"
press_button right
expect_full_text 'Reject?'
press_button both
expect_apdu_return $ERR_REJECT
press_button right
press_button right
press_button both
expect_exited
