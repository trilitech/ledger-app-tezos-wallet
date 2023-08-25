# full input: 0092abf8e3d9e5f8cfd9ae8a9fe5f28ea1d5b5abf1af82dae8a4b68df3d1889eb6f988f5e8d31a
# full output: 12345678901234567890123456789012345678901234567890123456789012345678901234567890
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
start_speculos "$seed"
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
send_apdu 800f000011048000002c800006c18000000080000000
expect_apdu_return 9000
send_apdu 800f810028050092abf8e3d9e5f8cfd9ae8a9fe5f28ea1d5b5abf1af82dae8a4b68df3d1889eb6f988f5e8d31a
expect_full_text 'Parsing error' 'ERR_TOO_LARGE'
press_button both
expect_apdu_return 9405
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
press_button right
expect_full_text 'Settings'
press_button right
expect_full_text 'Quit?'
press_button both
expect_exited
