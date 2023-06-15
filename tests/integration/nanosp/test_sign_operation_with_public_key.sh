# full input: 0300000000000000000000000000000000000000000000000000000000000000006b0275e3c3ca942b8cc8cd5ba379171a2ea8ed835eb880db35b905b206ea0301039e6981ec780e4bb847c9cd27dd66da79ba97a0e38f846a621e7d2ddc3b2f0eba
# signer: tz2GB5YHqF4UzQ8GP5yUqdhY9oVWRXCY2hPU
send_async_apdus \
	800f000111048000002c800006c18000000080000000 "expect_apdu_return 9000
"\
	800f8101620300000000000000000000000000000000000000000000000000000000000000006b0275e3c3ca942b8cc8cd5ba379171a2ea8ed835eb880db35b905b206ea0301039e6981ec780e4bb847c9cd27dd66da79ba97a0e38f846a621e7d2ddc3b2f0eba "check_tlv_signature_from_sent_apdu c5f3c0c74752f6350cd25623b835d8b415b564c1cb04b15bff3d45e575b90414 9000 sppk7bVy617DmGvXsMqcwsiLtnedTN2trUi5ugXcNig7en4rHJyunK1 0300000000000000000000000000000000000000000000000000000000000000006b0275e3c3ca942b8cc8cd5ba379171a2ea8ed835eb880db35b905b206ea0301039e6981ec780e4bb847c9cd27dd66da79ba97a0e38f846a621e7d2ddc3b2f0eba
"
expect_full_text 'Operation (0)' 'Reveal'
press_button right
expect_full_text 'Fee' '0.88 tz'
press_button right
expect_full_text 'Storage limit' '490'
press_button right
expect_full_text 'Public key' 'sppk7cT9TUns1TubQwkjZ3f75ehmtkhWit9eH4VvPvEiNtziaQL1'
press_button right
expect_full_text 'Accept?' 'Press both buttons' 'to accept.'
press_button both
expect_async_apdus_sent