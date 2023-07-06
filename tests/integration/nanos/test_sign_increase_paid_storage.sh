# full input: 00000000000000000000000000000000000000000000000000000000000000007100ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040501000000000000000000000000000000000000000000
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
send_async_apdus \
	800f000011048000002c800006c18000000080000000 "expect_apdu_return 9000" \
	800f8100530300000000000000000000000000000000000000000000000000000000000000007100ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040501000000000000000000000000000000000000000000 "expect_apdu_return 48ab3de08b4a53ffe8cb8984cb5e0174082496321d5f9644ec8a3f01f3b2176f6782addb600b8b796195b2591e4f240091d478214661ff6728b392f0a84d8eca66ddcb5d5c83838642c0335f7d6b797c835ea8f465550e8c1d5ec64d87193b079000"
expect_section_content nanos 'Operation (0)' 'Increase paid storage'
press_button right
expect_section_content nanos 'Fee' '0.01 tz'
press_button right
expect_section_content nanos 'Storage limit' '4'
press_button right
expect_section_content nanos 'Amount' '5'
press_button right
expect_section_content nanos 'Destination' 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT'
press_button right
expect_full_text 'Accept?'
press_button both
expect_async_apdus_sent
