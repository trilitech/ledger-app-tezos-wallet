# full input: 0300000000000000000000000000000000000000000000000000000000000000007200c921d4487c90b4472da6cc566a58d79f0d991dbf904e02030400747884d9abdf16b3ab745158925f567e222f71225501826fa83347f6cbe9c393
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
expect_full_text 'Tezos Wallet' 'ready for' 'safe signing'
send_async_apdus \
	800f000011048000002c800006c18000000080000000 "expect_apdu_return 9000
"\
	800f81005d0300000000000000000000000000000000000000000000000000000000000000007200c921d4487c90b4472da6cc566a58d79f0d991dbf904e02030400747884d9abdf16b3ab745158925f567e222f71225501826fa83347f6cbe9c393 "expect_apdu_return f9570b3272e25bc3a9c17a489547dcae70ae750adaa73b4b8eb5dd0b55be5987dd9c53607303e7c5f9c11488d5c8977d79e248f59f753eabf5ab7babfa61b1f6279b2e16dacb6db87e4e05cbdc23156d3e95989161d322ba2feb369beeb7b5049000
"
expect_section_content nanosp 'Operation (0)' 'Set consensus key'
press_button right
expect_section_content nanosp 'Fee' '0.01 tz'
press_button right
expect_section_content nanosp 'Storage limit' '4'
press_button right
expect_section_content nanosp 'Public key' 'edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY'
press_button right
expect_full_text 'Accept?' 'Press both buttons' 'to accept.'
press_button both
expect_async_apdus_sent
