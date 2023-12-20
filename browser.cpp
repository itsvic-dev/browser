#include "libdom/node.h"
#include "libhtml.h"
#include <cassert>
#include <cstdio>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>

LibHTML::Parser parser;

size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  (void)userdata;
  (void)size;
  assert(size == 1);
  parser.parse(ptr, nmemb);
  return nmemb;
}

void walkTree(std::shared_ptr<LibDOM::Node> node, int indent = 0) {
  std::cout << std::string(indent * 2, ' ') << "└" << node->internalName()
            << "\n";
  for (auto child : node->childNodes) {
    walkTree(child, indent + 1);
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <url>\n";
    return 1;
  }
  curl_global_init(CURL_GLOBAL_ALL);
  auto *handle = curl_easy_init();

  curl_easy_setopt(handle, CURLOPT_URL, argv[1]);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
  auto success = curl_easy_perform(handle);
  if (success != CURLE_OK) {
    std::cout << "failed to fetch: " << curl_easy_strerror(success) << "\n";
  }
  // let the tokenizer know we're EOF'd now
  const wchar_t eof[] = {EOF};
  parser.parse(eof, 1);

  std::cout << "\nDOM tree dump:\n";
  walkTree(parser.document);

  return 0;
}
