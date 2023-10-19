start_speculos "$seed"
sleep 0.2
expected_home
press_button right
expect_full_text "Settings"
press_button right
expect_full_text "Quit?"
press_button left
expect_full_text "Settings"
press_button left
expected_home
press_button both
expected_home
press_button right
expect_full_text "Settings"
press_button right
expect_full_text "Quit?"
press_button right
expect_full_text "Quit?"
press_button both
expect_exited
