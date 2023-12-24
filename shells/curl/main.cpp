#include "libdom/node.h"
#include "libdomrenderer/renderer.h"
#include "libdomrenderer/viewport.h"
#include "libhtml.h"
#include "version.h"
#include <cassert>
#include <cstdio>
#include <curl/curl.h>
#include <curl/easy.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>

LibHTML::Parser parser;

size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  (void)userdata;
  (void)size;
  assert(size == 1);
  try {
    parser.parse(ptr, nmemb);
  } catch (std::exception &exc) {
    std::cout
        << "[FATAL ERROR] An exception has occurred during LibHTML parsing: "
        << exc.what() << "\n";
    return -1;
  }
  return nmemb;
}

void walkTree(std::shared_ptr<LibDOM::Node> node, int indent = 0) {
  std::wcout << std::wstring(indent * 2, ' ') << L"-> "
             << node->internalName().c_str();
  if (!node->nodeName.empty()) {
    std::wcout << " (" << node->nodeName << ")";
  }
  std::wcout << "\n";
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
  if (!handle)
    return -2;

  curl_easy_setopt(handle, CURLOPT_URL, argv[1]);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Accept: text/html; charset=UTF-8");
  headers = curl_slist_append(headers, "User-Agent: " BROWSER_USER_AGENT);
  curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);

  auto success = curl_easy_perform(handle);
  if (success != CURLE_OK) {
    return -1;
  }
  curl_slist_free_all(headers);

  // let the tokenizer know we're EOF'd now
  const wchar_t eof[] = {EOF};
  parser.parse(eof, 1);

  std::cout << "\nDOM tree dump:\n";
  walkTree(parser.document);

  // render tree
  auto viewport = std::make_shared<LibDOMRenderer::Viewport>(800, 600);
  LibDOMRenderer::Renderer renderer;
  renderer.renderToViewport(parser.document, viewport);

  // output ppm
  std::ofstream ppm("test.ppm");
  ppm << "P3\n"
      << viewport->getWidth() << " " << viewport->getHeight() << "\n255\n";

  for (size_t i = 0; i < viewport->getWidth() * viewport->getHeight() * 3;
       i++) {
    uint8_t color = viewport->getBuffer()[i];
    ppm << (int)color << "\n";
  }

  ppm.flush();
  return 0;
}
