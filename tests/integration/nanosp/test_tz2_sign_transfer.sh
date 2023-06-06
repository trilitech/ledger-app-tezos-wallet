# full input: 0300000000000000000000000000000000000000000000000000000000000000006c000000000000000000000000000000000000000000a0c21ed802ad05b601a0c21e01000000000000000000000000000000000000000000ffff086a65616e5f626f62000000020345
# full output: SIZE
# signer: tz2GB5YHqF4UzQ8GP5yUqdhY9oVWRXCY2hPU
send_async_apdus \
        800f000111048000002c800006c18000000080000000 9000\
        800f81016a0300000000000000000000000000000000000000000000000000000000000000006c000000000000000000000000000000000000000000a0c21ed802ad05b601a0c21e01000000000000000000000000000000000000000000ffff086a65616e5f626f62000000020345 ""
expect_full_text 'Operation (0)' 'Transaction'
press_button right
expect_full_text 'Fee' '0.5 tz'
press_button right
expect_full_text 'Storage limit' '182'
press_button right
expect_full_text 'Amount' '0.5 tz'
press_button right
expect_full_text 'Destination' 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT'
press_button right
expect_full_text 'Entrypoint' 'jean_bob'
press_button right
# FIXME: OCR bug
expect_full_text 'Data' 'lZE'
press_button right
expect_full_text 'Accept?' 'Press both buttons' 'to accept.'
press_button both
expect_async_apdus_sent
