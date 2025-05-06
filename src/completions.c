#include "../include/completions.h"
#include "../include/globdef.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdio.h>
#include <string.h>

#define ENDPOINT_COMPLETIONS "https://api.openai.com/v1/chat/completions"

static unsigned long s_buff = 0;

static int write_func(void *ptr, int size, int nmemb, char output[]) {
  const unsigned long totalSize = size * nmemb;
  char *c_ptr = (char *)ptr;
  memcpy(&output[s_buff], ptr, s_buff + totalSize);
  s_buff += totalSize;
  return totalSize;
}

const int get_prompt_response(const char api_key[], const char model[],
                              const char role[], const char instruction[],
                              const char input[], char output[],
                              const int outputSize) {
  CURL *pCurl = curl_easy_init();
  if (pCurl == NULL)
    return 1;

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
  snprintf(
      data, sizeof(data),
      "{ \"model\": \"%s\", \"messages\": [{ \"role\": \"%s\", "
      "\"content\": \"%s\" }, { \"role\": \"user\", \"content\": \"%s\" }] }",
      model, role, instruction, input);
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
