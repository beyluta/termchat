#include "completions.h"
#include "globdef.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static constexpr uint8_t ENDPOINT_COMPLETIONS[] =
    "https://api.openai.com/v1/chat/completions";
static constexpr uint8_t MAX_CONTEXT_ARRAY_SIZE = 255;
static char context[MAX_CONTEXT_ARRAY_SIZE][MAX_BUFF_SIZE];
static uint16_t context_size = 0;
static size_t s_buff = 0;
static volatile bool g_request_pending = false;

typedef struct {
  CURL *curl;
  CURLcode code;
} request_info_t;

typedef struct {
  size_t length;
  char string[MAX_BUFF_SIZE];
} string_t;

/**
 * @brief Creates a string on the stack containing length and the pointer
 * @param srcDest Result where the string will be saved
 * @param string String input to create
 */
static void create_string(string_t *const srcDest, const char *const string) {
  const size_t length = strlen(string);
  memcpy(srcDest->string, string, length);
  srcDest->string[length] = '\0';
  srcDest->length = length;
}

/**
 * @brief Gets the correct role string based on the type
 * @param role Numeric representation of the role type
 * @param dest Where the string will be stored
 * @returns The status of the operation
 */
static size_t get_role_type(const role_type_t role, char *const dest) {
  string_t string = {};
  switch (role) {
  default:
    return ERR_UNRECOVERABLE;
  case role_type_user:
    create_string(&string, "user");
    break;
  case role_type_assistant:
    create_string(&string, "assistant");
    break;
  case role_type_developer:
    create_string(&string, "developer");
  }
  memcpy(dest, string.string, string.length);
  return ERR_RECOVERABLE;
}

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
 * @param role_type Role of the current message
 * @return A static constant integer representing the result of the operation.
 */
size_t add_context(const char *const input, role_type_t role_type) {
  if (context_size >= MAX_CONTEXT_ARRAY_SIZE) {
    fprintf(stderr, "Context window limit has been exceeded\n");
    return ERR_UNRECOVERABLE;
  }

  char role[MAX_BUFF_SIZE] = {};
  if (get_role_type(role_type, role) == ERR_UNRECOVERABLE) {
    fprintf(stderr, "Role failed to resolve\n");
    return ERR_UNRECOVERABLE;
  }

  char temp[MAX_BUFF_SIZE] = {};
  const char template[] = "{\"role\":\"%s\",\"content\":\"%s\"}";
  if (snprintf(temp, MAX_BUFF_SIZE, template, role, input) < 0) {
    fprintf(stderr, "Input could not be added to context\n");
    return ERR_UNRECOVERABLE;
  }

  memcpy(context[context_size], temp, MAX_BUFF_SIZE);
  context[context_size++][MAX_BUFF_SIZE - 1] = '\0';
  return ERR_RECOVERABLE;
}

/**
 * @brief Get the timestamp of the current date
 * @returns The timestamp in seconds, or -1 on error
 */
static ssize_t date_now() { return (unsigned long)time(nullptr); }

/**
 * @brief Thread that will take care of processing the Rest API request
 * @param src The arguments of the function
 */
static void *on_request_processing(void *src) {
  request_info_t *info = (request_info_t *)src;
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

  g_request_pending = true;
  request_info_t info = {
      .code = status,
      .curl = pCurl,
  };
  pthread_t thread;
  if (pthread_create(&thread, nullptr, on_request_processing, &info) != 0) {
    fprintf(stderr, "Failed to create new thread\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  ssize_t timestamp = date_now();
  if (timestamp < 0) {
    fprintf(stderr, "Timestamp could not be fetched outside while\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  while (g_request_pending) {
    const ssize_t currentTime = date_now();
    if (currentTime < 0) {
      fprintf(stderr, "Timestamp failed while request was pending\n");
      status = ERR_UNRECOVERABLE;
      goto cleanup;
    }

    if (currentTime >= timestamp) {
      printf(".");
      if (fflush(stdout) != 0) {
        fprintf(stderr, "Failed to write unwritten bytes to stdout\n");
        status = ERR_UNRECOVERABLE;
        goto cleanup;
      }
      timestamp = currentTime + 1;
    }
  }
  printf("\n");

  if (info.code != CURLE_OK) {
    fprintf(stderr, "Request failed or could not be sent to the endpoint\n");
    status = ERR_UNRECOVERABLE;
    goto cleanup;
  }

  output[s_buff] = '\0';
  s_buff = 0;

cleanup:
  if (pHeaders != nullptr) {
    curl_slist_free_all(pHeaders);
  }

  if (pCurl != nullptr) {
    curl_easy_cleanup(pCurl);
  }

  if (pthread_detach(thread) != 0) {
    fprintf(stderr, "Thread could not be detached\n");
    status = ERR_UNRECOVERABLE;
  }
  return status;
}
