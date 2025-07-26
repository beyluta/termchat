#ifndef WINDOW_H
#define WINDOW_H

#include <stddef.h>

/**
 * @struct Window
 * @brief Object containing window properties
 */
struct Window {
  const size_t width;
  const size_t height;
  const size_t title_size;
  const char *content;
  const char *title;
} typedef Window;

/**
 * @brief Clears the terminal window.
 * @return An integer status code indicating success or failure.
 */
int clear_chat_window();

/**
 * @brief Draws the inner chat window along with context
 * @param window Properties of the window to draw
 * @return Whether the function was successful
 */
int draw_chat_window(const Window window);

/**
 * @brief Calculate dimensions of the window with respect to context
 * @param input Context string to be rendered inside the window
 * @param title Title of the window
 * @return Object containing window properties
 */
Window get_window_properties(const char input[], const char title[]);
#endif
