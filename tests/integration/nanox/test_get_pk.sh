# Address expected: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
start_speculos "$seed"
sleep 0.2
expect_full_text "Tezos Wallet" "ready for" "safe signing"
send_apdu 8002000011048000002c800006c18000000080000000
expect_apdu_return 2102747884d9abdf16b3ab745158925f567e222f71225501826fa83347f6cbe9c3939000
press_button right
press_button right
press_button both
expect_exited
