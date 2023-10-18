start_speculos "$seed"
sleep 0.2
expected_home
send_apdu 8000000000
expect_apdu_return "$VERSION_BYTES""9000"
quit_app
