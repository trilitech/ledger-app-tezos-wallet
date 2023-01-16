#pragma once

#include <stdbool.h>

#define TZ_UI_STREAM_HISTORY_SCREENS  4
#define TZ_UI_STREAM_TITLE_WIDTH      TZ_SCREEN_WITDH_BETWEEN_ICONS_BOLD_11PX
#define TZ_UI_STREAM_CONTENTS_WIDTH   TZ_SCREEN_WITDH_FULL_REGULAR_11PX
#define TZ_UI_STREAM_CONTENTS_LINES   (TZ_SCREEN_LINES_11PX - 1)

#define TZ_UI_STREAM_CONTENTS_SIZE (TZ_UI_STREAM_CONTENTS_WIDTH * TZ_UI_STREAM_CONTENTS_LINES)

typedef struct {
  char titles[TZ_UI_STREAM_HISTORY_SCREENS][TZ_UI_STREAM_TITLE_WIDTH];
  char values[TZ_UI_STREAM_HISTORY_SCREENS][TZ_UI_STREAM_CONTENTS_SIZE];
  char title[TZ_UI_STREAM_TITLE_WIDTH + 1];
  char value[TZ_UI_STREAM_CONTENTS_SIZE + 1];
  int16_t current;
  int16_t total;
  bool full;
} tz_ui_stream_model;

typedef struct {
  char title[TZ_UI_STREAM_TITLE_WIDTH + 1];
  char value[TZ_UI_STREAM_CONTENTS_SIZE + 1];
} tz_ui_stream_buffer;

typedef enum {
  TZ_UI_STREAM_DISPLAY_INIT,
  TZ_UI_STREAM_DISPLAY_FIRST,
  TZ_UI_STREAM_DISPLAY_CANNOT_GO_BACK,
  TZ_UI_STREAM_DISPLAY_CONT,
  TZ_UI_STREAM_DISPLAY_ACCEPT,
  TZ_UI_STREAM_DISPLAY_REJECT,
} tz_ui_stream_screen_kind;

typedef struct {
  void (*refill)();
  void (*accept)();
  void (*reject)();
} tz_ui_stream_callbacks;

void tz_ui_stream_init (void (*refill)(), void (*accept)(), void (*reject)());
void tz_ui_stream_push ();
void tz_ui_stream_close ();
tz_ui_stream_screen_kind tz_ui_stream_current_screen_kind ();
__attribute__((noreturn)) void tz_ui_stream ();
