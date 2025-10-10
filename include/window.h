#ifndef WINDOW_H
#define WINDOW_H

#include "globdef.h"
#include <stddef.h>

/**
 * @struct Window
 * @brief Object containing window properties
 */
typedef struct {
  size_t width;
  size_t height;
  size_t title_size;
  char content[MAX_BUFF_SIZE];
  char title[MAX_BUFF_SIZE];
} window_t;

/**
 * @brief Clears the terminal window.
 * @return An integer status code indicating success or failure.
 */
size_t clear_chat_window();

/**
 * @brief Draws the inner chat window along with context
 * @param window Properties of the window to draw
 * @return Whether the function was successful
 */
size_t draw_chat_window(const window_t window);

/**
 * @brief Calculate dimensions of the window with respect to context
 * @param input Context string to be rendered inside the window
 * @param title Title of the window
 * @return The status of the operation
 */
size_t get_window_properties(const char *const input, const char *const title,
                             window_t *const dest);
#endif
