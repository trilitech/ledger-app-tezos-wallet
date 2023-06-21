# full input: 0092abf8e3d9e5f8cfd9ae8a9fe5f28ea1d5b5abf1af82dae8a4b68df3d1889eb6f988f5e8d31a
# full output: 12345678901234567890123456789012345678901234567890123456789012345678901234567890
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
press_button right
expect_full_text 'Tezos Wallet' 'ready for' 'BLIND signing'
send_async_apdus \
	800f000011048000002c800006c18000000080000000 "expect_apdu_return 9000" \
	800f810028050092abf8e3d9e5f8cfd9ae8a9fe5f28ea1d5b5abf1af82dae8a4b68df3d1889eb6f988f5e8d31a "expect_apdu_return ef565fa445d815cd77518a4d14ce90b7a536627455f0930c9dbfa22a75d478d83e2bcb333ba0d639dd28c1b77c5860e552ab02092a50a57f1424f573278230ab8ba81d8a40956415278a27e3f28cae64d1f1f13bf613e6e9a57035e9e14511029000"
expect_section_content nanosp 'Sign Hash (1/3)' 'H7Gq5omPmhFhDPeyywQ'
press_button right
expect_section_content nanosp 'Sign Hash (2/3)' 'QsSb5Mmt2fiBwsgwv'
press_button right
expect_section_content nanosp 'Sign Hash (3/3)' 'vuNXzu18Xq'
press_button right
expect_full_text 'Accept?' 'Press both buttons to accept.'
press_button both
expect_async_apdus_sent
