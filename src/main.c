#include "../include/completions.h"
#include "../include/config.h"
#include "../include/globdef.h"
#include "../include/window.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

constexpr unsigned char COMMAND_MIN_LEN = 2;
constexpr char COMMAND_DELIMITER = '`';
constexpr char HELP_TABLE[] = "+----------------+----------------------+-------"
                              "--------------------------+\n"
                              "| Short-form     | Long-form            | "
                              "Purpose                         |\n"
                              "+----------------+----------------------+------"
                              "---------------------------+\n"
                              "| -i             | --interactive        | "
                              "Enters interactive mode         |\n"
                              "| -h             | --help               | Shows "
                              "a table with all commands |\n"
                              "+----------------+----------------------+------"
                              "---------------------------+\n";

struct Parameters {
  bool interactive_mode;
  bool help_mode;
} typedef Parameters;

static size_t get_parameters(const int argc, const char **argv,
                             Parameters *params) {
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
      params->interactive_mode = true;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      params->help_mode = true;
    }
  }
  return ERR_RECOVERABLE;
}

static size_t unescape_string(char *input, const char match) {
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

static size_t get_executable_command(const char *src, char *dest) {
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

static size_t event_loop(const char **argv, const Parameters *params) {
  if (params->interactive_mode == true)
    clear_chat_window();

  bool print_model = true;

  for (;;) {
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
    if (get_rc_contents(filepath, config, sizeof(config)) ==
        ERR_UNRECOVERABLE) {
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

    char prompt_input[MAX_BUFF_SIZE];
    if (params->interactive_mode == true) {
      if (print_model) {
        printf("(%s)> ", model);
      }

      fgets(prompt_input, sizeof(prompt_input), stdin);
      prompt_input[strcspn(prompt_input, "\n")] = 0;
      print_model = true;
    }

    const size_t input_len = strlen(prompt_input);
    if (input_len <= 0) {
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
    const int content_size = get_json_value(prompt_output, "content", content);
    if (content_size <= 0) {
      fprintf(stderr,
              "Could not parse JSON response into a readable format. Attempted "
              "to parse %s\n",
              prompt_output);
      return ERR_UNRECOVERABLE;
    }
    content[content_size] = '\0';

    if (add_context(content, false) == ERR_UNRECOVERABLE) {
      fprintf(stderr, "Could not capture response to window context\n");
      return ERR_UNRECOVERABLE;
    }

    unescape_string(content, '"');
    const Window window = get_window_properties(content, model);
    draw_chat_window(window);

    char command[MAX_BUFF_SIZE];
    if (get_executable_command(content, command) == ERR_RECOVERABLE) {
      printf("> %s would like to execute (Y/n): %s\n", model, command);
      const char next_char = getchar();
      if (next_char == 'y' || next_char == 'Y') {
        FILE *file = popen(command, "r");
        if (file == nullptr) {
          fprintf(stderr, "Failed to execute command\n");
        }
        char output[MAX_BUFF_SIZE];
        while (fgets(output, MAX_BUFF_SIZE, file) != NULL) {
          printf("%s", output);
        }
        pclose(file);
      }
    }

    if (params->interactive_mode == false)
      return ERR_RECOVERABLE;
  }

  return ERR_UNRECOVERABLE;
}

int main(const int argc, const char **argv) {
  Parameters params = {};
  get_parameters(argc, argv, &params);

  if (params.help_mode == true) {
    printf("%s", HELP_TABLE);
    return ERR_RECOVERABLE;
  }

  if (argc < 2 && params.interactive_mode == false) {
    fprintf(stderr, "Error: Invalid arguments.\n");
    fprintf(
        stderr,
        "Usage: ./<PROG_NAME> \"how to create a file via the terminal?\"\n");
    return ERR_UNRECOVERABLE;
  }
  return event_loop(argv, &params);
}
