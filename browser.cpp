#include "libhtml.h"
#include <cassert>
#include <cstdio>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>

LibHTML::Tokenizer tokenizer;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  (void)userdata;
  (void)size;
  assert(size == 1);
  tokenizer.process(ptr, nmemb);
  return tokenizer.processed();
}

int main() {
  curl_global_init(CURL_GLOBAL_ALL);
  auto handle = curl_easy_init();

  curl_easy_setopt(handle, CURLOPT_URL, "https://example.com");
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
  auto success = curl_easy_perform(handle);
  if (success != CURLE_OK) {
    std::cout << "failed to fetch: " << curl_easy_strerror(success) << "\n";
  }
  // let the tokenizer know we're EOF'd now
  const char eof[] = {EOF};
  tokenizer.process(eof, 1);

  return 0;
}
