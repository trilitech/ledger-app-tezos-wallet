screen_1="Tezos Walletready forsafe signing"
screen_2="Quit?"
sleep 0.2
expect_full_text "$screen_1"
press_button right
expect_full_text "$screen_2"
press_button left
expect_full_text "$screen_1"
press_button left
expect_full_text "$screen_1"
press_button both
expect_full_text "$screen_1"
press_button right
expect_full_text "$screen_2"
press_button right
expect_full_text "$screen_2"
press_button both
expect_exited
