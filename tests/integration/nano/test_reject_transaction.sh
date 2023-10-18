# full input: 0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316
# full output: CAR
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
# path: m/44'/1729'/0'/0'
start_speculos "$seed"
expect_full_text "Tezos Wallet" "ready for" "safe signing"
send_async_apdus \
	800f000011048000002c800006c18000000080000000 "expect_apdu_return 9000
"\
	800f81005e0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316 "expect_apdu_return $ERR_REJECT
"
expect_full_text 'Operation (0)' 'Transaction'
press_button right
expect_full_text 'Fee' '0.05 tz'
press_button right
expect_full_text 'Storage limit' '45'
press_button right
expect_full_text 'Amount' '0.24 tz'
press_button right
expect_section_content nanox 'Destination' 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT'
press_button right
expect_full_text 'Entrypoint' 'do'
press_button right
expect_section_content nanox 'Parameter' 'CAR'
press_button right
expect_full_text 'Accept?' 'Press both buttons to accept.'
press_button right
expect_full_text 'Reject?' 'Press both buttons to reject.'
press_button both
expect_async_apdus_sent
