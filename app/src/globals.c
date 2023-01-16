#include "globals.h"

globals_t global;

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;

void clear_apdu_globals(void) {
    memset(&global.apdu, 0, sizeof(global.apdu));
}

void init_globals(void) {
    memset(&global, 0, sizeof(global));
    memset(G_io_seproxyhal_spi_buffer, 0, sizeof(G_io_seproxyhal_spi_buffer));
    memset(&G_ux, 0, sizeof(G_ux));
    memset(&G_ux_params, 0, sizeof(G_ux_params));
}
