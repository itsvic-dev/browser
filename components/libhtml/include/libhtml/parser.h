#ifndef LIBHTML_PARSER_H
#define LIBHTML_PARSER_H

#include "libdom.h"
#include "libdom/element.h"
#include "libhtml/tokenizer.h"
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

class Parser {
public:
  Parser();

  void parse(const char *text, size_t textLen);
  void parse(const wchar_t *text, size_t textLen);

private:
  void onEmit(std::unique_ptr<Token> token);
  Tokenizer m_tokenizer;
};

} // namespace LibHTML

#endif
