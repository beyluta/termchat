#include "../include/completions.h"
#include "../include/config.h"
#include "../include/globdef.h"
#include "../include/window.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Error: Invalid arguments.\n");
    fprintf(
        stderr,
        "Usage: ./<PROG_NAME> \"how to create a file via the terminal?\"\n");
    return ERR_UNRECOVERABLE;
  }

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
  if (get_json_value(config, "openai", api_key) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Failed to get the api key from the config file\n");
    return ERR_UNRECOVERABLE;
  }

  char model[MAX_BUFF_SIZE];
  if (get_json_value(config, "model", model) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Failed to get gpt model from config file\n");
    return ERR_UNRECOVERABLE;
  }

  char role[MAX_BUFF_SIZE];
  if (get_json_value(config, "role", role) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Failed to get role from config file\n");
    return ERR_UNRECOVERABLE;
  }

  char instruction[MAX_BUFF_SIZE];
  if (get_json_value(config, "instruction", instruction) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Failed to get instruction from config file\n");
    return ERR_UNRECOVERABLE;
  }

  char prompt_output[MAX_CHAT_BUFF_CONTEXT];
  if (get_prompt_response(api_key, model, role, instruction, argv[1],
                          prompt_output,
                          sizeof(prompt_output)) == ERR_UNRECOVERABLE) {
    fprintf(stderr,
            "Could not get a response from the OpenAI Completions API\n");
    return ERR_UNRECOVERABLE;
  }

  char content[MAX_CHAT_BUFF_CONTEXT];
  if (get_json_value(prompt_output, "content", content) == ERR_UNRECOVERABLE) {
    fprintf(stderr,
            "Could not parse JSON response into a readable format. Attempted "
            "to parse %s\n",
            prompt_output);
    return ERR_UNRECOVERABLE;
  }

  const Window window = calc_input_window_dimensions(content);
  if (draw_chat_window(window) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Could not draw the content to the terminal window.\n");
    return ERR_UNRECOVERABLE;
  }
  return ERR_RECOVERABLE;
}
