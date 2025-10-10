#include "../include/completions.h"
#include "../include/globdef.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <string.h>

constexpr char ENDPOINT_COMPLETIONS[] =
    "https://api.openai.com/v1/chat/completions";
constexpr char ROLE_USER[] = "user";
constexpr char ROLE_ASSISTANT[] = "assistant";
constexpr unsigned char MAX_CONTEXT_ARRAY_SIZE = 255;

static char context[MAX_CONTEXT_ARRAY_SIZE][MAX_BUFF_SIZE];
static unsigned short context_size = 0;
static unsigned long s_buff = 0;

static size_t get_context(char *const dest) {
  size_t start = 0;
  for (int i = 0; i < context_size; i++) {
    const size_t contextLength = strlen(context[i]) + 1;

    // If we don't memcpy this into a local array, GCC will refuse to compile
    char userContext[MAX_BUFF_SIZE];
    memcpy(userContext, context[i], MAX_BUFF_SIZE);
    userContext[MAX_CONTEXT_ARRAY_SIZE - 1] = '\0';

    char temp[MAX_BUFF_SIZE + 1];
    if (snprintf(temp, MAX_BUFF_SIZE + 1, "%s,", userContext) < 0) {
      fprintf(stderr, "User context could not be written into temp variable\n");
      return ERR_UNRECOVERABLE;
    }

    if (contextLength >= MAX_BUFF_SIZE) {
      fprintf(stderr, "Context was bigger than maximum allowed\n");
      return ERR_UNRECOVERABLE;
    }

    memcpy(&dest[start], temp, contextLength);
    start += contextLength;
  }
  dest[start - 1] = '\0';
  return ERR_RECOVERABLE;
}

static size_t write_func(void *const ptr, size_t size, size_t nmemb,
                         void *const output) {
  const size_t totalSize = size * nmemb;
  char *outputPtr = (char *)output;
  memcpy(&outputPtr[s_buff], ptr, s_buff + totalSize);
  s_buff += totalSize;
  return totalSize;
}

/**
 * @brief Adds context based on the provided input.
 * @param input The input string to process.
 * @param is_user A boolean indicating if the context is user-specific.
 * @return A static constant integer representing the result of the operation.
 */
size_t add_context(const char *const input, bool is_user) {
  if (context_size >= MAX_CONTEXT_ARRAY_SIZE) {
    fprintf(stderr, "Context window limit has been exceeded\n");
    return ERR_UNRECOVERABLE;
  }

  char temp[MAX_BUFF_SIZE] = {};
  snprintf(temp, MAX_BUFF_SIZE, "{\"role\":\"%s\",\"content\":\"%s\"}",
           is_user == true ? ROLE_USER : ROLE_ASSISTANT, input);
  memcpy(context[context_size], temp, MAX_BUFF_SIZE);
  context[context_size++][MAX_BUFF_SIZE - 1] = '\0';
  return ERR_RECOVERABLE;
}

/**
 * @brief Makes a call to the OpenAI completions API and receives the response
 * of the LLM. The ouput is saved to the argument of the same name and contains
 * the raw text context of the reply.
 *
 * @param api_key OpenAI generated api key
 * @param model GPT model to use
 * @param role Role of the LLM: 'system' | 'user' | 'developer' | 'tool' |
 * 'assistant'
 * @param instruction instruction on what the LLM should do
 * @param input user input
 * @param output output buffer to be written to
 * @param outputSize size of the buffer
 * @return Whether the function was successful
 */
size_t get_prompt_response(const char *const api_key, const char *const model,
                           const char *const role,
                           const char *const instruction,
                           const char *const input, char *const output) {
  CURL *pCurl = curl_easy_init();
  if (pCurl == NULL) {
    fprintf(stderr, "Could not initialize libcurl\n");
    return ERR_UNRECOVERABLE;
  }

  if (add_context(input, true) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Could not add context to window\n");
    return ERR_UNRECOVERABLE;
  }

  char chat_ctx[MAX_USR_SIZE];
  if (get_context(chat_ctx) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Error reading entire chat context\n");
    return ERR_UNRECOVERABLE;
  }

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

  char data[MAX_BUFF_SIZE] = {};
  snprintf(data, MAX_BUFF_SIZE,
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
