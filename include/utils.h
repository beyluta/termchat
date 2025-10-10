#ifndef UTILS_H
#define UTILS_H

#include "globdef.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

typedef enum : uint8_t {
  term_color_none,
  term_color_green,
  term_color_red,
} term_color_t;

typedef struct {
  size_t length;
  char text[MAX_BUFF_SIZE];
} term_string_t;

#define term_print_color(a, b)                                                 \
  _Generic((a),                                                                \
      char *: term_print_color_char,                                           \
      term_string_t: term_print_color_string)(a, b)

/**
 * @brief Merges a list of strings into a single contiguous string
 * @param string String to hold the entire data
 * @param argc Count of how many strings there are
 * @returns The status of the operation
 */
static size_t merge_strings(term_string_t *const string, size_t argc, ...) {
  va_list args;
  va_start(args, argc);

  size_t status = ERR_RECOVERABLE;
  for (size_t i = 0; i < argc; i++) {
    const ssize_t written = snprintf(&string->text[string->length],
                                     MAX_BUFF_SIZE, "%s", va_arg(args, char *));
    if (written < 0) {
      fprintf(stderr, "Strings could not be merged\n");
      status = ERR_UNRECOVERABLE;
      goto cleanup;
    }

    string->length += written;
  }

cleanup:
  va_end(args);
  return status;
}

/**
 * @brief Prints a text in a predefined color to stdout
 * @param src Text to print
 * @param color Color to use
 */
static void term_print_color_char(const char *const src, term_color_t color) {
  switch (color) {
  default:
  case term_color_none:
    printf("%s\n", src);
    break;
  case term_color_green:
    printf("\033[32m%s\033[0m\n", src);
    break;
  case term_color_red:
    fprintf(stderr, "\033[31m%s\033[0m\n", src);
    break;
  }
}

/**
 * @brief Prints a text in a predefined color to stdout
 * @param src Text to print
 * @param color Color to use
 */
static void term_print_color_string(const term_string_t src,
                                    term_color_t color) {
  switch (color) {
  default:
  case term_color_none:
    printf("%s\n", src.text);
    break;
  case term_color_green:
    printf("\033[32m%s\033[0m\n", src.text);
    break;
  case term_color_red:
    fprintf(stderr, "\033[31m%s\033[0m\n", src.text);
    break;
  }
}

#endif
