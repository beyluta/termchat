#include "../include/window.h"
#include "../include/globdef.h"
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static const int draw_top_bottom_border_window(const int width) {
  for (int i = 0; i <= width; i++)
    printf("%c", i <= 0 || i >= width ? '+' : '-');
  return ERR_RECOVERABLE;
}

static const int get_terminal_window_size() {
  struct winsize window;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window) < 0) {
    fprintf(stderr, "Could not determine terminal window size\n");
    return 0;
  }
  return window.ws_col - 1;
}

static const int draw_text_window(const int width, const int height,
                                  const char text[]) {
  const int window_size = get_terminal_window_size() - 2;
  for (int j = 0, k = 0; j < height; j++) {
    printf("\n");
    for (int i = 0; i <= window_size; i++) {
      if (i != 0 && i != window_size) {
        if (k < width) {
          printf("%c", text[k++]);
        } else {
          printf(" ");
        }
      } else if (i <= 0) {
        printf("| ");
      } else if (i >= window_size) {
        printf(" |");
      }
    }
  }
  return ERR_RECOVERABLE;
}

const int clear_chat_window() {
  printf("\e[1;1H\e[2J");
  return ERR_RECOVERABLE;
}

const int draw_chat_window(const Window window) {
  const int window_size = get_terminal_window_size();
  clear_chat_window();
  draw_top_bottom_border_window(window_size);
  draw_text_window(window.title_size, 1, window.title);
  printf("\n");
  draw_top_bottom_border_window(window_size);
  draw_text_window(window.width, window.height, window.content);
  printf("\n");
  draw_top_bottom_border_window(window_size);
  printf("\n");
  return ERR_RECOVERABLE;
}

const Window get_window_properties(const char input[], const char title[]) {
  const int window_size = get_terminal_window_size();
  const int width = strlen(input);
  const int height = (width / window_size) + 1;
  const Window window = {.height = height,
                         .width = width,
                         .content = input,
                         .title = title,
                         .title_size = strlen(title)};
  return window;
}
