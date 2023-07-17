# full input: 0000000000000000000000000000000000000000000000000000000000000000c900ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040000001100000008530a0a530a530a530000000153
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
start_speculos "$seed"
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
send_async_apdus \
	800f000011048000002c800006c18000000080000000 "expect_apdu_return 9000" \
	800f810051030000000000000000000000000000000000000000000000000000000000000000c900ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040000001100000008530a0a530a530a530000000153 "expect_apdu_return a8c59c87d4161cf2d1286da325b6b4ef387693ef6c870ebe2998c40d6f8f1b5f02479a0fc53201f263eae719339e0e4881ae853bd2d4a2bead2688af79f6c4b4681e87ab40423f2fe4a69b7ccdd91377a503cb2094d25772c95d9ca32bf63f079000"
expect_section_content nanosp 'Operation (0)' 'SR: send messages'
press_button right
expect_section_content nanosp 'Fee' '0.01 tz'
press_button right
expect_section_content nanosp 'Storage limit' '4'
press_button right
expect_section_content nanosp 'Message (0)' 'S

S
S
S'
press_button right
expect_section_content nanosp 'Message (1)' 'S'
press_button right
expect_full_text 'Accept?' 'Press both buttons to accept.'
press_button both
expect_async_apdus_sent
