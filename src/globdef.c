#include "../include/globdef.h"
#include "../jsmn/jsmn.h"
#include <string.h>

#define MAX_JSON_TOKENS 128

static const int get_token_index(void *tokens, const char input[],
                                 const int index, char output[]) {
  const jsmntok_t *token_array = (jsmntok_t *)tokens;
  const int len = token_array[index].end - token_array[index].start;
  strncpy(output, &input[token_array[index].start], len);
  output[len] = '\0';
  return ERR_RECOVERABLE;
}

const int get_json_value(const char input[], const char key[], char output[]) {
  jsmn_parser parser;
  jsmntok_t tokens[MAX_JSON_TOKENS];

  jsmn_init(&parser);
  int token_count =
      jsmn_parse(&parser, input, strlen(input), tokens, MAX_JSON_TOKENS);
  if (token_count < 0)
    return ERR_UNRECOVERABLE;

  for (int i = 0; i < token_count; i++) {
    char token[4096];
    get_token_index(tokens, input, i, token);

    if (strcmp(token, key) == 0) {
      get_token_index(tokens, input, i + 1, output);
      return ERR_RECOVERABLE;
    }
  }

  return ERR_UNRECOVERABLE;
}
