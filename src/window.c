#include "../include/window.h"
#include "../include/globdef.h"
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**
 * @brief Draws the top and bottom borders of the pseudo window
 * @param width Horizontal size of the border to draw to
 * @returns The status of the operation
 */
static size_t draw_top_bottom_border_window(const int width) {
  for (int i = 0; i <= width; i++)
    printf("%c", i <= 0 || i >= width ? '+' : '-');
  return ERR_RECOVERABLE;
}

/**
 * @brief Gets the max size of the terminal window
 *  @returns Size of the terminal window
 */
static size_t get_terminal_window_size() {
  struct winsize window;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window) < 0) {
    fprintf(stderr, "Could not determine terminal window size\n");
    return 0;
  }
  return window.ws_col - 1;
}

/**
 * @brief Draws a text inside the pseudo window
 * @param width Horizontal size of the window
 * @param height Vertical size of the window
 * @param text Content to be printed to stdout
 * @returns The status of the operation
 */
static size_t draw_text_window(const int width, const int height,
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

/**
 * @brief Clears the chat window
 * @retursn The status of the operation
 */
size_t clear_chat_window() {
  printf("\e[1;1H\e[2J");
  return ERR_RECOVERABLE;
}

/**
 * @brief Draws a pseudo window inside the terminal
 * @retursn The status of the operation
 */
size_t draw_chat_window(const window_t window) {
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

/**
 * @brief Draws a pseudo window inside the terminal
 * @retursn The status of the operation
 */
size_t get_window_properties(const char *const input, const char *const title,
                             window_t *dest) {
  const int window_size = get_terminal_window_size();
  const int width = strlen(input);
  const int height = (width / window_size) + 1;

  dest->height = height;
  dest->width = width;
  dest->title_size = strlen(title);

  const size_t inputLen = strlen(input);
  const size_t contentLen = strlen(title);

  if (snprintf(dest->content, inputLen, "%s", input) < 0) {
    fprintf(stderr, "Failed to move string into correct memory location\n");
    return ERR_UNRECOVERABLE;
  }

  if (snprintf(dest->title, contentLen, "%s", title) < 0) {
    fprintf(stderr, "Failed to move string into correct memory location\n");
    return ERR_UNRECOVERABLE;
  }

  return ERR_RECOVERABLE;
}
