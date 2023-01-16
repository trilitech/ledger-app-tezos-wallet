#pragma once

#include "os_io_seproxyhal.h"
#include "os.h"
#include "ux.h"

#define DISPLAY(elts, cb)\
  memcpy(global.ux.bagls, elts, sizeof(elts)); \
  G_ux.stack[0].element_arrays[0].element_array = global.ux.bagls; \
  G_ux.stack[0].element_arrays[0].element_array_count = sizeof(elts)/sizeof(bagl_element_t); \
  G_ux.stack[0].button_push_callback = cb; \
  G_ux.stack[0].screen_before_element_display_callback = NULL; \
  UX_WAKE_UP(); \
  UX_REDISPLAY();

#define REGULAR BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER
#define BOLD BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER

extern const bagl_icon_details_t C_icon_rien;
void ui_initial_screen(void);
__attribute__((noreturn)) bool exit_app(void);
