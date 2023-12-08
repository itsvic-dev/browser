#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>

int main() {
  curl_global_init(CURL_GLOBAL_ALL);
  auto handle = curl_easy_init();

  curl_easy_setopt(handle, CURLOPT_URL, "https://example.com");
  auto success = curl_easy_perform(handle);
  if (success != CURLE_OK) {
    std::cout << "failed to fetch:" << curl_easy_strerror(success) << "\n";
  }

  return 0;
}
