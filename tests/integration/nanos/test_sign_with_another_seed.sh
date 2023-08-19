# full input: 00000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316
# signer: tz1aex9GimxigprKi8MQK3j6sSisDpbtRBXw
# path: m/17'/8'/6'/9'
start_speculos "around dignity equal spread between young lawsuit interest climb wide that panther rather mom snake scene ecology reunion ice illegal brush"
expect_full_text 'ready for' 'safe signing'
send_async_apdus \
	800f0000110480000011800000088000000680000009 "expect_apdu_return 9000" \
	800f81005e0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316 "expect_apdu_return f6d5fa0e79cac216e25104938ac873ca17ee9d7f06763719293b413cf2ed475c858379f820b144f87d3ac95011564620a650460081904a18d783cfab08d05d4dce0456a111bf3a64732120dcef5a89991fc93420dacda1809043d541dcff130b9000"
expect_section_content nanos 'Operation (0)' 'Transaction'
press_button right
expect_section_content nanos 'Fee' '0.05 tz'
press_button right
expect_section_content nanos 'Storage limit' '45'
press_button right
expect_section_content nanos 'Amount' '0.24 tz'
press_button right
expect_section_content nanos 'Destination' 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT'
press_button right
expect_section_content nanos 'Entrypoint' 'do'
press_button right
expect_section_content nanos 'Parameter' 'CAR'
press_button right
expect_full_text 'Accept?'
press_button both
expect_async_apdus_sent
