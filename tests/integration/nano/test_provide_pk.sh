start_speculos "$seed"
sleep 0.2
expected_home
send_apdu 8003000011048000002c800006c18000000080000000
expect_full_text "Provide Key" 'tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E'
press_button right
expected_accept
press_button both
expect_apdu_return 2102747884d9abdf16b3ab745158925f567e222f71225501826fa83347f6cbe9c3939000
quit_app