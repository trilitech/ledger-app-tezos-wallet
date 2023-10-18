start_speculos "$seed"
sleep 0.2
expect_full_text "Tezos Wallet" "ready for" "safe signing"
press_button right
expect_full_text "Settings"
press_button right
expect_full_text "Quit?"
press_button left
expect_full_text "Settings"
press_button left
expect_full_text "Tezos Wallet" "ready for" "safe signing"
press_button both
expect_full_text "Tezos Wallet" "ready for" "safe signing"
press_button right
expect_full_text "Settings"
press_button right
expect_full_text "Quit?"
press_button right
expect_full_text "Quit?"
press_button both
expect_exited
