start_speculos "$seed"
sleep 0.2
expected_home
send_apdu 8003000311048000002c800006c18000000080000000
# bip25519 derivations take longer to compute, especially on CI platform.
# Thus, additional TIMEOUT is needed.
TIMEOUT=80 expect_section_content "Provide Key" 'tz1VKyZ3RFDwTkrz5LKcTc6fcYqZj6pvsyA7'
press_button right
expected_accept
press_button both
TIMEOUT=100 expect_apdu_return 210293c6b359964a4332bf1355579d665b753343f7b0a42567978cea1671f7b89f479000
quit_app
