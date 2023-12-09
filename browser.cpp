#include "libhtml.h"
#include "libhtml/dom/node.h"
#include <cassert>
#include <cstdio>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>

LibHTML::Tokenizer tokenizer;
LibHTML::ASTParser parser;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  (void)userdata;
  (void)size;
  assert(size == 1);
  tokenizer.process(ptr, nmemb);
  return tokenizer.processed();
}

void walkTree(std::shared_ptr<LibHTML::DOM::Node> node, int indent = 0) {
  std::cout << std::string(indent * 2, ' ') << "└" << node->_name() << "\n";
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
  auto handle = curl_easy_init();

  curl_easy_setopt(handle, CURLOPT_URL, argv[1]);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
  auto success = curl_easy_perform(handle);
  if (success != CURLE_OK) {
    std::cout << "failed to fetch: " << curl_easy_strerror(success) << "\n";
  }
  // let the tokenizer know we're EOF'd now
  const char eof[] = {EOF};
  tokenizer.process(eof, 1);
  parser.parse(tokenizer.tokens);

  std::cout << "\n\nDOM tree dump:\n";
  walkTree(parser.document);

  return 0;
}
