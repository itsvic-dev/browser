#pragma once
#include "libhtml/dom.h"
#include "libhtml/dom/element.h"
#include "libhtml/tokens.h"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace LibHTML {

#define NS_HTML L"http://www.w3.org/1999/xhtml"

enum ParserMode {
  INITIAL,
  BEFORE_HTML,
  BEFORE_HEAD,
  IN_HEAD,
  IN_HEAD_NOSCRIPT,
  AFTER_HEAD,
  IN_BODY,
  TEXT,
  IN_TABLE,
  IN_TABLE_TEXT,
  IN_CAPTION,
  IN_COLUMN_GROUP,
  IN_TABLE_BODY,
  IN_ROW,
  IN_CELL,
  IN_SELECT,
  IN_SELECT_IN_TABLE,
  IN_TEMPLATE,
  AFTER_BODY,
  IN_FRAMESET,
  AFTER_FRAMESET,
  AFTER_AFTER_BODY,
  AFTER_AFTER_FRAMESET,
};

class ASTParser {
public:
  ASTParser();

  void parse(std::vector<std::shared_ptr<Token>> tokens);
  std::shared_ptr<DOM::Document> document;

private:
  void parseTick();
  void consume();

  std::shared_ptr<DOM::HTMLElement>
  createElement(std::shared_ptr<DOM::Document> document, std::wstring localName,
                std::wstring ns, std::wstring prefix = L"");

  std::shared_ptr<DOM::HTMLElement>
  createElementForToken(std::shared_ptr<TagToken> token);

  size_t tokenPtr = 0;
  std::shared_ptr<Token> token = nullptr;
  std::vector<std::shared_ptr<Token>> tokens = {};

  ParserMode insertionMode = INITIAL;
  bool fosterParenting = false;

  std::vector<std::shared_ptr<DOM::Element>> openElementStack;
};

} // namespace LibHTML
