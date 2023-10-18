# full input: 00000000000000000000000000000000000000000000000000000000000000000600ffdd6102321bc251e4a5190ad5b12b251069d9b4000000200bcd7b2cadcd87ecb0d5c50330fb59feed7432bffecede8a09a2b86cfb33847b00
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
start_speculos "$seed"
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
send_async_apdus \
	800f000011048000002c800006c18000000080000000 "expect_apdu_return 9000" \
	800f81005c0300000000000000000000000000000000000000000000000000000000000000000600ffdd6102321bc251e4a5190ad5b12b251069d9b4000000200bcd7b2cadcd87ecb0d5c50330fb59feed7432bffecede8a09a2b86cfb33847b00 "expect_apdu_return 80a74079a1911a95b4aea45d6b29321e1705165194521bf74982c4b8c576824088a4217edccca5dd9d12295666a489c6d8939a71a9b8b6116f2fa62f0cb7c5f08a0bc975f3103a2063a6a18dafa5313a0f627453f11d06ac47b049aed60b0e079000"
expect_section_content nanox 'Operation (0)' 'Ballot'
press_button right
expect_section_content nanox 'Source' 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa'
press_button right
expect_section_content nanox 'Period' '32'
press_button right
expect_section_content nanox 'Proposal' 'ProtoALphaALphaALphaALphaALphaALphaALpha61322gcLUGH'
press_button right
expect_section_content nanox 'Ballot' 'yay'
press_button right
expect_full_text 'Accept?' 'Press both buttons to accept.'
press_button both
expect_async_apdus_sent
