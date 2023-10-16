# original operation : 0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316

# Unknown magic bytes
start_speculos "$seed"
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
send_apdu 800f000011048000002c800006c18000000080000000
expect_apdu_return 9000
send_apdu 800f81005e0100000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316
expect_full_text 'Parsing error' 'ERR_INVALID_TAG'
press_button both
expect_apdu_return 9405

# Unknown operation
send_apdu 800f000011048000002c800006c18000000080000000
expect_apdu_return 9000
send_apdu 800f81005e03000000000000000000000000000000000000000000000000000000000000000001016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316
expect_full_text 'Parsing error' 'ERR_INVALID_TAG'
press_button both
expect_apdu_return 9405

# 1 byte remove inside
send_apdu 800f000011048000002c800006c18000000080000000
expect_apdu_return 9000
send_apdu 800f81005d0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e010000000000000000000000000000000000000000ff02000000020316
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
expect_full_text 'Entrypoint' 'default'
press_button right
expect_full_text 'Parsing error' 'ERR_INVALID_TAG'
press_button both
expect_apdu_return 9405

# 1 byte introduce at the end
send_apdu 800f000011048000002c800006c18000000080000000
expect_apdu_return 9000
send_apdu 800f81005f0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff0200000002031645
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
expect_full_text 'Parsing error' 'ERR_INVALID_TAG'
press_button both
expect_apdu_return 9405

# 1 byte introduce inside
send_apdu 800f000011048000002c800006c18000000080000000
expect_apdu_return 9000
send_apdu 800f81005f0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e0100000000000000000000000000000000000000000000ff02000000020316
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
expect_full_text 'Parsing error' 'ERR_INVALID_TAG'
press_button both
expect_apdu_return 9405
press_button right
press_button right
press_button both
expect_exited
