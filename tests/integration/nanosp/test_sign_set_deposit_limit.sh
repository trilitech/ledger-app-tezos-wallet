# full input: 03000000000000000000000000000000000000000000000000000000000000000070027c252d3806e6519ed064026bdb98edf866117331e0d40304f80204ffa09c01
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
start_speculos "$seed"
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
send_async_apdus \
	800f000011048000002c800006c18000000080000000 "expect_apdu_return 9000
"\
	800f81004203000000000000000000000000000000000000000000000000000000000000000070027c252d3806e6519ed064026bdb98edf866117331e0d40304f80204ffa09c01 "expect_apdu_return 8b4456454de1b3c41f5ea45e711893df26fabe9427048b95fda4276d5cf76ff6069c2cd9fe167a52cc21611a0f59465784f4fce94211aab9fee6309c8e8bf5cbcf1a3e3102d0825b5acaf341656b1c2078850f7d3a6749cc47f74688fbe2c30e9000
"
expect_section_content nanosp 'Operation (0)' 'Set deposit limit'
press_button right
expect_section_content nanosp 'Fee' '0.06 tz'
press_button right
expect_section_content nanosp 'Storage limit' '4'
press_button right
expect_section_content nanosp 'Staking limit' '0.02 tz'
press_button right
expect_full_text 'Accept?' 'Press both buttons to accept.'
press_button both
expect_async_apdus_sent
