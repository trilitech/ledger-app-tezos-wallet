# full input: 00000000000000000000000000000000000000000000000000000000000000006d00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304a0c21e0000000002037a0000000a07650100000001310002
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
send_async_apdus \
	800f000011048000002c800006c18000000080000000 "expect_apdu_return 9000" \
	800f8100540300000000000000000000000000000000000000000000000000000000000000006d00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304a0c21e0000000002037a0000000a07650100000001310002 "expect_apdu_return 4c6b124e010e5701ea67043d176136234a1dc9c3514dbc512209bd0a28033dcf621fcc5d36f84f3fffd9177278499f16bb363ae4573227105fd584f58d1be77ebcfc17250cf575b8aad31951f18e35a3252f10583768abf4f2511bd84669c0039000"
expect_section_content nanosp 'Operation (0)' 'Origination'
press_button right
expect_section_content nanosp 'Fee' '0.01 tz'
press_button right
expect_section_content nanosp 'Storage limit' '4'
press_button right
expect_section_content nanosp 'Balance' '0.5 tz'
press_button right
expect_section_content nanosp 'Delegate' 'Field unset'
press_button right
expect_section_content nanosp 'Code' 'UNPAIR'
press_button right
expect_section_content nanosp 'Storage' 'pair "1" 2'
press_button right
expect_full_text 'Accept?' 'Press both buttons to accept.'
press_button both
expect_async_apdus_sent
