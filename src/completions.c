#include "../include/completions.h"
#include "../include/globdef.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <string.h>

#define ENDPOINT_COMPLETIONS "https://api.openai.com/v1/chat/completions"
#define MAX_CONTEXT_SIZE 256
#define ROLE_USER "user"
#define ROLE_ASSISTANT "assistant"

static char context[MAX_CONTEXT_SIZE][MAX_BUFF_SIZE];
static unsigned short context_size = 0;
static unsigned long s_buff = 0;

static const int get_context(char dest[], const int size) {
  int start = 0;
  for (int i = 0; i < context_size; i++) {
    const int ctx_size = strlen(context[i]) + 1;
    char temp[MAX_BUFF_SIZE];
    snprintf(temp, ctx_size + 1, "%s,", context[i]);
    memcpy(&dest[start], temp, ctx_size);
    start += ctx_size;
  }
  dest[start - 1] = '\0';
  return ERR_UNRECOVERABLE;
}

static int write_func(void *ptr, int size, int nmemb, char output[]) {
  const unsigned long totalSize = size * nmemb;
  char *c_ptr = (char *)ptr;
  memcpy(&output[s_buff], ptr, s_buff + totalSize);
  s_buff += totalSize;
  return totalSize;
}

const int add_context(const char input[], BOOLEAN is_user) {
  if (context_size >= MAX_CONTEXT_SIZE) {
    fprintf(stderr, "Context window limit has been exceeded\n");
    return ERR_UNRECOVERABLE;
  }

  const int temp_len =
      strlen(input) + 28 + strlen(is_user == TRUE ? ROLE_USER : ROLE_ASSISTANT);
  char temp[MAX_BUFF_SIZE];
  snprintf(temp, temp_len, "{\"role\":\"%s\",\"content\":\"%s\"}",
           is_user == TRUE ? ROLE_USER : ROLE_ASSISTANT, input);

  memcpy(context[context_size], temp, MAX_BUFF_SIZE);
  context[context_size++][temp_len] = '\0';
  return ERR_RECOVERABLE;
}

const int get_prompt_response(const char api_key[], const char model[],
                              const char role[], const char instruction[],
                              const char input[], char output[]) {
  CURL *pCurl = curl_easy_init();
  if (pCurl == NULL) {
    fprintf(stderr, "Could not initialize libcurl\n");
    return ERR_UNRECOVERABLE;
  }

  if (add_context(input, TRUE) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Could not add context to window\n");
    return ERR_UNRECOVERABLE;
  }

  char chat_ctx[MAX_BUFF_SIZE];
  get_context(chat_ctx, sizeof(chat_ctx));

  curl_easy_setopt(pCurl, CURLOPT_URL, ENDPOINT_COMPLETIONS);

  struct curl_slist *pHeaders = NULL;
  pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");

  char authorization[MAX_BUFF_SIZE];
  snprintf(authorization, sizeof(authorization), "Authorization: Bearer %s",
           api_key);
  pHeaders = curl_slist_append(pHeaders, authorization);
  curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
  curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_func);
  curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, output);

  char data[MAX_BUFF_SIZE];
  snprintf(data, sizeof(data),
           "{ \"model\": \"%s\", \"messages\": [{ \"role\": \"%s\", "
           "\"content\": \"%s\" }, %s] }",
           model, role, instruction, chat_ctx);
  curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, data);

  const CURLcode response = curl_easy_perform(pCurl);
  if (response != CURLE_OK)
    strcpy(output, "Could not complete request");

  output[s_buff] = '\0';
  s_buff = 0;
  curl_slist_free_all(pHeaders);
  curl_easy_cleanup(pCurl);
  return response != CURLE_OK ? ERR_UNRECOVERABLE : ERR_RECOVERABLE;
}
