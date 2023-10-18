start_speculos "$seed"
sleep 0.2
expected_home
send_apdu 8009000000
expect_apdu_return "$COMMIT_BYTES""00""9000"
quit_app
