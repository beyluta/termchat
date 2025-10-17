#include "completions.h"
#include "config.h"
#include "globdef.h"
#include "utils.h"
#include <signal.h>
#include <stdint.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

constexpr uint8_t ARG_FLAG_POSITION = 1;
constexpr uint8_t COMMAND_MIN_LEN = 2;
constexpr uint8_t COMMAND_DELIMITER = '`';
constexpr uint8_t HELP_TABLE[] =
    "+----------------+---------------------------------+\n"
    "| Short-form     | Purpose                         |\n"
    "+----------------+---------------------------------+\n"
    "| -i             | Enters interactive mode         |\n"
    "| -h             | Shows a table with all commands |\n"
    "+----------------+---------------------------------+\n";

typedef struct {
  bool interactive_mode;
  bool help_mode;
} term_params_t;

typedef enum : uint8_t {
  term_flag_none,
  term_flag_help,
  term_flag_interactive
} term_flag_t;

typedef enum : uint8_t {
  term_code_newline = 10,
  term_code_space = 32
} term_code_t;

static volatile bool g_keep_alive = true;

/**
 * @brief Get the code for the specific parameter
 * @param src The name of the parameter
 * @returns The code of the argument passed by string
 */
static term_flag_t get_flag_code(const char *const src) {
  term_flag_t status = term_flag_none;
  status += !!(strcmp(src, "-i") == 0) * term_flag_interactive;
  status += !!(strcmp(src, "-h") == 0) * term_flag_help;
  return status;
}

/**
 * @brief Get the state of the parameters inside a struct
 * @param argc Count of arguments the program started with
 * @param argv Array of arguments
 * @param params struct which will store the state of the parameters
 */
static void get_parameters(const int argc, const char *const *argv,
                           term_params_t *const params) {
  if (argc <= ARG_FLAG_POSITION) {
    return;
  }

  const char *const param = argv[ARG_FLAG_POSITION];
  const term_flag_t argument = get_flag_code(param);
  switch (argument) {
  default:
  case term_flag_none:
    // No need to do anything here; just process the prompt normally
    break;
  case term_flag_help:
    params->help_mode = true;
    break;
  case term_flag_interactive:
    params->interactive_mode = true;
    break;
  }
}

/**
 * @brief Unescape a string passed by argument
 * @param input String to unescape
 * @param match delimiter to look out for
 * @returns The status of the operation
 */
static size_t unescape_string(char *const input, const char match) {
  int length = strlen(input);
  for (int i = 0; i < length; i++) {
    if (input[i] == '\\' && i + 1 < length && input[i + 1] == match) {
      memcpy(&input[i], &input[i + 1], --length);
      i--;
      input[length] = '\0';
    }
  }
  return ERR_RECOVERABLE;
}

/**
 * @brief Get a substring inside a string into the dest pointer
 * @param src Source string which contains the entire content
 * @param dest Destination string where the substring will reside
 * @returns The status of the operation
 */
static size_t get_executable_command(const char *const src, char *const dest) {
  const int len = strlen(src);
  int start = -1, end = -1;
  for (int i = 0; i < len; i++) {
    if (src[i] == COMMAND_DELIMITER) {
      if (start < 0) {
        start = i;
        continue;
      }

      end = i;
      break;
    }
  }

  const int size = end - start;
  if (start < 0 || end < 0 || size > MAX_BUFF_SIZE || size < COMMAND_MIN_LEN) {
    return ERR_UNRECOVERABLE;
  }

  memcpy(dest, &src[start + 1], size - 1);
  dest[size - 1] = '\0';
  return ERR_RECOVERABLE;
}

/**
 * @brief Replaces all instances of a char with another in a string
 * @param string String to analyse and modify
 * @param target The char that should be replaced
 * @param repalce The new char to be inserted
 */
static void replace_chars_in_string(term_string_t *const string,
                                    const term_code_t target,
                                    const term_code_t replace) {
  for (size_t i = 0; i < string->length; i++) {
    if (string->text[i] == target) {
      string->text[i] = replace;
    }
  }
}

/**
 * @brief Searches and executes the first instance of a command inside the src
 * string. A command is defined as being any substring between two backticks
 * (e.g. `mkdir build`)
 *
 * @param src The source string containing the command within
 * @param model String containing the name of the LLM model
 */
static size_t process_string_command(const char *const src,
                                     const char *const model) {
  char command[MAX_BUFF_SIZE];
  if (get_executable_command(src, command) == ERR_RECOVERABLE) {
    term_string_t string = {.length = 0};
    const char *const condition = " would like to execute (Y/n): ";
    if (merge_strings(&string, 4, "> ", model, condition, command) ==
        ERR_UNRECOVERABLE) {
      fprintf(stderr, "Failed to merge strings to show executable command\n");
      return ERR_UNRECOVERABLE;
    }

    term_print_color(string, term_color_red);

    // Process the next keypress without needing to press enter
    struct termios old_termios, new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    const char next_char = getchar();
    if (next_char == 'y' || next_char == 'Y') {
      FILE *file = nullptr;
      if ((file = popen(command, "r")) == nullptr) {
        fprintf(stderr, "Failed to execute command\n");
        return ERR_UNRECOVERABLE;
      }

      char output[MAX_BUFF_SIZE];
      while (fgets(output, MAX_BUFF_SIZE, file) != NULL) {
        printf("%s", output);
      }

      term_string_t str = {.length = 0};
      if (merge_strings(&str, 2, "Resource:", output) == ERR_UNRECOVERABLE) {
        fprintf(stderr, "Failed to merge resource with command string\n");
        return ERR_UNRECOVERABLE;
      }

      replace_chars_in_string(&str, term_code_newline, term_code_space);

      if (add_context(str.text, role_type_developer) == ERR_UNRECOVERABLE) {
        fprintf(stderr, "Command could not be added to context history\n");
        return ERR_UNRECOVERABLE;
      }

      pclose(file);
    }

    // Reverting the changes made to the terminal above
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
  }

  return ERR_RECOVERABLE;
}

/**
 * @brief Clears the terminal window
 */
static void clear_terminal() { printf("\e[1;1H\e[2J"); }

/**
 * @brief Used to detect when a signal interrupt is sent
 * @param int Number of the signal received
 */
static void on_sigint_received(int) {
  g_keep_alive = false;
  clear_terminal();
  exit(0);
}

/**
 * @brief Get the next line from stdin
 * @param dest Destination pointer where the string will be saved
 */
static size_t get_next_line(char *const dest, const size_t len) {
  if (fgets(dest, len, stdin) == nullptr) {
    fprintf(stderr, "Failed to read next line\n");
    return ERR_UNRECOVERABLE;
  }
  dest[strcspn(dest, "\n")] = 0;
  return ERR_RECOVERABLE;
}

/**
 * @brief Event loop of the entire application if started with the '-i' flag
 * @param argv Array of string arguments
 * @param params Struct containing all parameters of the application
 * @returns The status of the operation
 */
static size_t event_loop(const char *const *argv,
                         const term_params_t *const params) {
  if (params->interactive_mode == true) {
    clear_terminal();
    signal(SIGINT, on_sigint_received);
  }

  bool print_model = true;
  char filepath[MAX_BUFF_SIZE];

  if (get_rc_path(filepath, MAX_BUFF_SIZE) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Failed to get config file directory\n");
    return ERR_UNRECOVERABLE;
  }

  if (get_rc_exists() == FILE_NOT_EXISTS) {
    fprintf(stderr, "Could not find rc file at %s\n", filepath);
    return ERR_UNRECOVERABLE;
  }

  char config[MAX_BUFF_SIZE];
  if (get_rc_contents(filepath, config, sizeof(config)) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Failed to get config file contents\n");
    return ERR_UNRECOVERABLE;
  }

  char api_key[MAX_BUFF_SIZE];
  if (get_json_value(config, "openai", api_key) <= 0) {
    fprintf(stderr, "Failed to get the api key from the config file\n");
    return ERR_UNRECOVERABLE;
  }

  char model[MAX_BUFF_SIZE];
  const int model_size = get_json_value(config, "model", model);
  if (model_size <= 0) {
    fprintf(stderr, "Failed to get gpt model from config file\n");
    return ERR_UNRECOVERABLE;
  }

  char role[MAX_BUFF_SIZE];
  if (get_json_value(config, "role", role) <= 0) {
    fprintf(stderr, "Failed to get role from config file\n");
    return ERR_UNRECOVERABLE;
  }

  char instruction[MAX_BUFF_SIZE];
  if (get_json_value(config, "instruction", instruction) <= 0) {
    fprintf(stderr, "Failed to get instruction from config file\n");
    return ERR_UNRECOVERABLE;
  }

  while (g_keep_alive) {
    char prompt_input[MAX_BUFF_SIZE] = {};
    if (params->interactive_mode) {
      if (print_model) {
        printf("(%s)> ", model);
      }

      if (get_next_line(prompt_input, MAX_BUFF_SIZE)) {
        fprintf(stderr, "Failed to put next line into buffer\n");
        return ERR_UNRECOVERABLE;
      }

      print_model = true;
    } else {
      if (snprintf(prompt_input, MAX_BUFF_SIZE, "%s", argv[1]) < 0) {
        fprintf(stderr, "Failed to fit argument into prompt input\n");
        return ERR_UNRECOVERABLE;
      }
    }

    const size_t inputLength = strlen(prompt_input);
    if (inputLength <= 0) {
      print_model = false;
      continue;
    }

    char prompt_output[MAX_BUFF_SIZE];
    if (get_prompt_response(api_key, model, role, instruction,
                            params->interactive_mode == false ? argv[1]
                                                              : prompt_input,
                            prompt_output) == ERR_UNRECOVERABLE) {
      fprintf(stderr,
              "Could not get a response from the OpenAI Completions API\n");
      return ERR_UNRECOVERABLE;
    }

    char content[MAX_BUFF_SIZE];
    if (!get_json_value(prompt_output, "content", content)) {
      fprintf(stderr,
              "Could not parse JSON response into a readable format. Attempted "
              "to parse %s\n",
              prompt_output);
      return ERR_UNRECOVERABLE;
    }

    if (add_context(content, role_type_assistant) == ERR_UNRECOVERABLE) {
      fprintf(stderr, "Could not capture response to window context\n");
      return ERR_UNRECOVERABLE;
    }

    if (unescape_string(content, '"') == ERR_UNRECOVERABLE) {
      fprintf(stderr, "Failed to unescape string\n");
      return ERR_UNRECOVERABLE;
    }

    term_string_t printableString = {.length = 0};

    if (merge_strings(&printableString, 2, content, "\n") ==
        ERR_UNRECOVERABLE) {
      fprintf(stderr, "Could not merge content string and newline\n");
      return ERR_UNRECOVERABLE;
    }
    term_print_color(printableString, term_color_green);

    if (process_string_command(content, model) == ERR_UNRECOVERABLE) {
      fprintf(stderr, "Could not process command\n");
      return ERR_UNRECOVERABLE;
    }

    if (params->interactive_mode == false) {
      return ERR_RECOVERABLE;
    }
  }

  return ERR_UNRECOVERABLE;
}

/**
 * @brief Entry point of the application
 * @param argc Number of arguments given at the start of the program
 * @param argv Array of strings given at the start of the program
 * @returns The status of the operation, 0 if success
 */
int main(const int argc, const char *const *argv) {
  term_params_t params = {};
  get_parameters(argc, argv, &params);

  if (params.help_mode == true) {
    printf("%s", HELP_TABLE);
    return ERR_RECOVERABLE;
  }

  if (argc < 2 && params.interactive_mode == false) {
    term_print_color_char("Error: Invalid arguments.", term_color_red);
    fprintf(
        stderr,
        "Usage: ./<PROG_NAME> \"how to create a file via the terminal?\"\n");
    return ERR_UNRECOVERABLE;
  }

  return event_loop(argv, &params);
}
