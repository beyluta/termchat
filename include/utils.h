#ifndef UTILS_H
#define UTILS_H

#include "globdef.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum : uint8_t {
  term_color_red = 31,
  term_color_green = 32,
  term_color_none = 37,
} term_color_t;

typedef enum : uint8_t {
  term_code_newline = 10,
  term_code_space = 32,
  term_code_backslash = 92
} term_code_t;

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
 * @brief Prints a string char by char
 * @param src Source string to print
 * @param len Length of the string
 * @param color Color of the string to print out
 */
static void custom_print_string(const char *const src, const size_t len,
                                const term_color_t color) {
  for (size_t i = 0; i < len; i++) {
    if (src[i] == term_code_backslash && ++i <= len && src[i] == 'n') {
      printf("\n");
      continue;
    }
    printf("\033[%dm%c\033[0m", color, src[i]);
  }
  printf("\n");
}

/**
 * @brief Prints a text in a predefined color to stdout
 * @param src Text to print
 * @param color Color to use
 */
static void term_print_color_char(const char *const src,
                                  const term_color_t color) {
  custom_print_string(src, strlen(src), color);
}

/**
 * @brief Prints a text in a predefined color to stdout
 * @param src Text to print
 * @param color Color to use
 */
static void term_print_color_string(const term_string_t src,
                                    const term_color_t color) {
  custom_print_string(src.text, src.length, color);
}

#endif
