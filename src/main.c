#include "../include/completions.h"
#include "../include/config.h"
#include "../include/globdef.h"
#include "../include/window.h"
#include <stdio.h>
#include <string.h>

const static int get_parameters(const int argc, const char **argv,
                                BOOLEAN *interactive_mode, BOOLEAN *help_mode) {
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
      *interactive_mode = TRUE;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      *help_mode = TRUE;
    }
  }
  return ERR_RECOVERABLE;
}

const static int event_loop(const int argc, const char **argv,
                            const BOOLEAN run_once) {
  if (run_once == FALSE)
    clear_chat_window();

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
    if (run_once == FALSE) {
      printf("(%s)> ", model);
      fgets(prompt_input, sizeof(prompt_input), stdin);
      prompt_input[strcspn(prompt_input, "\n")] = 0;
    }

    char prompt_output[MAX_CHAT_BUFF_CONTEXT];
    if (get_prompt_response(api_key, model, role, instruction,
                            run_once == TRUE ? argv[1] : prompt_input,
                            prompt_output,
                            sizeof(prompt_output)) == ERR_UNRECOVERABLE) {
      fprintf(stderr,
              "Could not get a response from the OpenAI Completions API\n");
      return ERR_UNRECOVERABLE;
    }

    char content[MAX_CHAT_BUFF_CONTEXT];
    const int content_size = get_json_value(prompt_output, "content", content);
    if (content_size <= 0) {
      fprintf(stderr,
              "Could not parse JSON response into a readable format. Attempted "
              "to parse %s\n",
              prompt_output);
      return ERR_UNRECOVERABLE;
    }

    const Window window =
        calc_input_window_dimensions(content, content_size, model, model_size);
    draw_chat_window(window);

    if (run_once == TRUE)
      return ERR_RECOVERABLE;
  }

  return ERR_UNRECOVERABLE;
}

int main(const int argc, const char **argv) {
  BOOLEAN interactive_mode = FALSE;
  BOOLEAN help_mode = FALSE;
  get_parameters(argc, argv, &interactive_mode, &help_mode);

  if (help_mode == TRUE) {
    const char help_table[] =
        "+----------------+-----------------------+------"
        "--------------------------+\n"
        "| Short-form     | Long-form            | "
        "Purpose                         |\n"
        "+----------------+------------------------+------"
        "-------------------------+\n"
        "| -i             | --interactive        | "
        "Enters interactive mode         |\n"
        "| -h             | --help               | Shows "
        "a table with all commands |\n"
        "+----------------+------------------------+------"
        "-------------------------+\n";
    printf("%s", help_table);
    return ERR_RECOVERABLE;
  }

  if (argc < 2 && interactive_mode == FALSE) {
    fprintf(stderr, "Error: Invalid arguments.\n");
    fprintf(
        stderr,
        "Usage: ./<PROG_NAME> \"how to create a file via the terminal?\"\n");
    return ERR_UNRECOVERABLE;
  }
  return event_loop(argc, argv, interactive_mode == FALSE);
}
