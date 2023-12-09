#pragma once
#include "libhtml/dom.h"
#include "libhtml/tokens.h"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace LibHTML {

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
  ASTParser() = default;

  void parse(std::vector<std::shared_ptr<Token>> tokens);

private:
  void parseTick();
  void consume();

  size_t tokenPtr = 0;
  std::shared_ptr<Token> token = nullptr;
  std::vector<std::shared_ptr<Token>> tokens = {};

  ParserMode insertionMode = INITIAL;
  DOM::Document document;
  bool fosterParenting = false;
};

} // namespace LibHTML
