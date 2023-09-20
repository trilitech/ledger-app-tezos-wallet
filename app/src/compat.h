/*
 * XXXrcd: Delete this file before the next release.
 * XXXrcd: Delete this file when its incorporated upstream:
 *         https://github.com/LedgerHQ/ledger-secure-sdk/pull/424
 *         it needs to be cherry-picked onto a fork for each arch
 *         so, it should take a couple of weeks
 *         NOTE: this function ignores the offsets because we aren't
 *               using them and we expect to delete this code very
 *               soon...
 */

#include <io.h>

static inline int
io_send_response_buffers(const buffer_t *rdatalist, size_t count, uint16_t sw)
{
    uint8_t buf[128];
    size_t  tx = 0;

    for (size_t i = 0; i < count; i++) {
        memcpy(&buf[tx], rdatalist[i].ptr, rdatalist[i].size);
        tx += rdatalist[i].size;
    }
    return io_send_response_pointer(buf, tx, sw);
}
