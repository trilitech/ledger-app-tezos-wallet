# full output: 0
# full input: 00000000000000000000000000000000000000000000000000000000000000006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304000156dcfb211fa76c525fd7c4566c09a5e3e4d5b81000ff01000000020000
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
send_async_apdus \
        800f000011048000002c800006c18000000080000000 "expect_apdu_return 9000" \
        800f81005b0300000000000000000000000000000000000000000000000000000000000000006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304000156dcfb211fa76c525fd7c4566c09a5e3e4d5b81000ff01000000020000 "expect_apdu_return 24aeac1f45f96ff13503b1354f8def563224633196aafb62e55df30c3894153f33496d120072845be599fd1811376ab93ec9f14130cb434d17ccc2563eb68b561b6c5e8b4893b588dea011cd0e2f49b992b750b78262cf318dd35e1945e87e009000"
expect_section_content nanos 'Operation (0)' 'Transaction'
press_button right
expect_section_content nanos 'Fee' '0.01 tz'
press_button right
expect_section_content nanos 'Storage limit' '4'
press_button right
expect_section_content nanos 'Amount' '0 tz'
press_button right
expect_section_content nanos 'Destination' 'KT1GW4QHn66m7WWWMWCMNaWmGYpCRbg5ahwU'
press_button right
expect_section_content nanos 'Entrypoint' 'root'
press_button right
expect_section_content nanos 'Parameter' '0'
press_button right
expect_full_text 'Accept?'
press_button both
expect_async_apdus_sent
