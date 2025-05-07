#ifndef WINDOW_H
#define WINDOW_H

/**
 * @struct Window
 * @brief Object containing window properties
 */
struct Window {
  const int width;
  const int height;
  const int title_size;
  const char *content;
  const char *title;
} typedef Window;

/**
 * @brief Clears the terminal window.
 * @return An integer status code indicating success or failure.
 */
const int clear_chat_window();

/**
 * @brief Draws the inner chat window along with context
 * @param window Properties of the window to draw
 * @return Whether the function was successful
 */
const int draw_chat_window(const Window window);

/**
 * @brief Calculate dimensions of the window with respect to context
 * @param input Context string to be rendered inside the window
 * @param window_title Title of the window
 * @return Object containing window properties
 */
const Window calc_input_window_dimensions(const char input[],
                                          const int input_size,
                                          const char title[],
                                          const int title_size);
#endif
