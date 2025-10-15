#include "completions.h"
#include "globdef.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

constexpr uint8_t ENDPOINT_COMPLETIONS[] =
    "https://api.openai.com/v1/chat/completions";
constexpr uint8_t ROLE_USER[] = "user";
constexpr uint8_t ROLE_ASSISTANT[] = "assistant";
constexpr uint8_t MAX_CONTEXT_ARRAY_SIZE = 255;

static char context[MAX_CONTEXT_ARRAY_SIZE][MAX_BUFF_SIZE];
static uint16_t context_size = 0;
static size_t s_buff = 0;

static volatile bool g_request_pending = false;

typedef struct {
  CURL *curl;
  CURLcode code;
} rest_thread_info_t;

/**
 * @brief Get the entire chat context from the current session
 * @param dest Pointer where the context will be saved to
 * @returns The status of the operation
 */
static size_t get_context(uint8_t *const dest) {
  size_t start = 0;
  for (uint16_t i = 0; i < context_size; i++) {
    char userContext[MAX_BUFF_SIZE];
    if (snprintf(userContext, MAX_BUFF_SIZE, "%s", context[i]) < 0) {
      fprintf(stderr, "Failed to copy context into local array\n");
      return ERR_UNRECOVERABLE;
    }

    char temp[MAX_BUFF_SIZE + 1];
    if (snprintf(temp, MAX_BUFF_SIZE + 1, "%s,", userContext) < 0) {
      fprintf(stderr, "User context could not be written into temp variable\n");
      return ERR_UNRECOVERABLE;
    }

    const size_t contextLength = strlen(context[i]) + 1;
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

/**
 * @brief Callback function that writes the response from the HTTP request into
 * the output pointer
 *
 * @param ptr
 * @param size
 * @param nmemb
 * @param ouput Buffer to save the http context to
 */
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
 * @brief Thread that will take care of processing the Rest API request
 * @param src The arguments of the function
 */
static void *on_request_processing(void *src) {
  rest_thread_info_t *info = (rest_thread_info_t *)src;
  if ((info->code = curl_easy_perform(info->curl)) != CURLE_OK) {
    fprintf(stderr, "Request failed from another thread\n");
  }
  g_request_pending = false;
  return nullptr;
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
  uint8_t status = ERR_RECOVERABLE;
  struct curl_slist *pHeaders = nullptr;
  CURL *pCurl = nullptr;

  if ((pCurl = curl_easy_init()) == nullptr) {
    fprintf(stderr, "Could not initialize libcurl\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  if (add_context(input, true) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Could not add context to window\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  uint8_t chat_ctx[MAX_USR_SIZE];
  if (get_context(chat_ctx) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Error reading entire chat context\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  curl_easy_setopt(pCurl, CURLOPT_URL, ENDPOINT_COMPLETIONS);

  pHeaders = curl_slist_append(pHeaders, "Content-Type: application/json");
  if (pHeaders == nullptr) {
    fprintf(stderr, "Content-Type was not set correctly\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  char authorization[MAX_BUFF_SIZE];
  if (snprintf(authorization, sizeof(authorization), "Authorization: Bearer %s",
               api_key) < 0) {
    fprintf(stderr, "API Key could not be added to authorization header\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  pHeaders = curl_slist_append(pHeaders, authorization);
  if (pHeaders == nullptr) {
    fprintf(stderr, "Could not add authorization header to http request\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  CURLcode curlStatus = CURLE_OK;
  if ((curlStatus = curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders)) !=
      CURLE_OK) {
    fprintf(stderr, "Could not set HTTP Header\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  if ((curlStatus = curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION,
                                     write_func)) != CURLE_OK) {
    fprintf(stderr, "Could not set function callback\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  if ((curlStatus = curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, output)) !=
      CURLE_OK) {
    fprintf(stderr, "Could not set write buffer to write data to\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  char data[MAX_BUFF_SIZE] = {};
  if (snprintf(data, MAX_BUFF_SIZE,
               "{ \"model\": \"%s\", \"messages\": [{ \"role\": \"%s\", "
               "\"content\": \"%s\" }, %s] }",
               model, role, instruction, chat_ctx) < 0) {
    fprintf(stderr, "Data buffer could not be built correctly\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  if ((curlStatus = curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, data)) !=
      CURLE_OK) {
    fprintf(stderr, "Failed to add json data to the request\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  // if ((status = curl_easy_perform(pCurl)) != CURLE_OK) {
  //   fprintf(stderr, "Request failed or could not be sent to the endpoint\n");
  //   status = ERR_UNRECOVERABLE;
  //   goto cleanup;
  // }

  g_request_pending = true;
  rest_thread_info_t info = {
      .code = status,
      .curl = pCurl,
  };
  pthread_t thread;
  pthread_create(&thread, nullptr, on_request_processing, &info);

  while (g_request_pending) {
    printf(".");
  }
  printf("\n");

  output[s_buff] = '\0';
  s_buff = 0;

cleanup:
  if (pHeaders != nullptr) {
    curl_slist_free_all(pHeaders);
  }

  if (pCurl != nullptr) {
    curl_easy_cleanup(pCurl);
  }
  return status;
}
