#ifndef WINDOW_H
#define WINDOW_H

/**
 * @struct Window
 * @brief Object containing window properties
 */
struct Window {
  const int width;
  const int height;
  const char *pStr;
} typedef Window;

/**
 * @brief Clears the terminal window
 * @return Whether the function was successful
 */
static const int clear_chat_window();

/**
 * @brief Draws a full length border from index 0 to width
 * @param width horizontal size of the border
 * @return Whether the function was successful
 */
static const int draw_top_bottom_border_window(const int width);

/**
 * @brief Draws the inner chat window along with context
 * @param window Properties of the window to draw
 * @return Whether the function was successful
 */
const int draw_chat_window(const Window window);

/**
 * @brief Calculate dimensions of the window with respect to context
 * @param input Context string to be rendered inside the window
 * @return Object containing window properties
 */
const Window calc_input_window_dimensions(const char input[]);

#endif
