#include "../include/window.h"
#include "../include/globdef.h"
#include <stdio.h>
#include <string.h>

#define MAX_CHAT_WINDOW_WIDTH 64

static const int clear_chat_window() {
  printf("\e[1;1H\e[2J");
  return ERR_RECOVERABLE;
}

static const int draw_top_bottom_border_window(const int width) {
  for (int i = 0; i <= width; i++)
    printf("%c", i <= 0 || i >= width ? '+' : '-');
  return ERR_RECOVERABLE;
}

const int draw_chat_window(const Window window) {
  if (window.width <= 4 || window.height <= 0 ||
      clear_chat_window() == ERR_UNRECOVERABLE)
    return ERR_UNRECOVERABLE;

  if (draw_top_bottom_border_window(MAX_CHAT_WINDOW_WIDTH) == ERR_UNRECOVERABLE)
    return ERR_UNRECOVERABLE;

  for (int j = 0, k = 0; j < window.height; j++) {
    printf("\n");
    for (int i = 0; i <= MAX_CHAT_WINDOW_WIDTH; i++) {
      if (i != 0 && i != MAX_CHAT_WINDOW_WIDTH) {
        if (k <= window.width)
          printf("%c", window.pStr[k++]);
        else
          printf(" ");
      } else if (i == 0 || i == MAX_CHAT_WINDOW_WIDTH)
        printf("|");
    }
  }

  printf("\n");
  if (draw_top_bottom_border_window(MAX_CHAT_WINDOW_WIDTH) == ERR_UNRECOVERABLE)
    return ERR_UNRECOVERABLE;

  printf("\n");
  return ERR_RECOVERABLE;
}

const Window calc_input_window_dimensions(const char input[]) {
  const int width = strlen(input) - 1;
  const int height = (width / MAX_CHAT_WINDOW_WIDTH) + 1;
  const Window window = {.height = height, .width = width, .pStr = input};
  return window;
}
