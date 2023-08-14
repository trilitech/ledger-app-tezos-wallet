# full input: 0000000000000000000000000000000000000000000000000000000000000000c900ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304000000140000000301234500000001670000000489abcdef
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
start_speculos "$seed"
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
send_async_apdus \
	800f000011048000002c800006c18000000080000000 "expect_apdu_return 9000" \
	800f810054030000000000000000000000000000000000000000000000000000000000000000c900ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304000000140000000301234500000001670000000489abcdef "expect_apdu_return 5ec1f51c235fecb7e66dd35acc31bf31a6fbc2aae1716ada0b953f3a95b91b6b0e996f8592454e899d66e62623249d4d7558b14f741a19df8b249446b8ec1a150c76c006791389e2dc3297faaaed0b3dc9a365de9beb1b5dd146b4855bd05b039000"
expect_section_content nanos 'Operation (0)' 'SR: send messages'
press_button right
expect_section_content nanos 'Fee' '0.01 tz'
press_button right
expect_section_content nanos 'Storage limit' '4'
press_button right
expect_section_content nanos 'Message (0)' '012345'
press_button right
expect_section_content nanos 'Message (1)' '67'
press_button right
expect_section_content nanos 'Message (2)' '89abcdef'
press_button right
expect_full_text 'Accept?'
press_button both
expect_async_apdus_sent
