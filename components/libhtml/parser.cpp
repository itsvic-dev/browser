#include "libhtml/parser.h"
#include "libhtml/exceptions.h"
#include <codecvt>
#include <exception>
#include <iostream>
#include <locale>
#include <string>

namespace LibHTML {

Parser::Parser() {}

void Parser::parse(const char *text, size_t textLen) {
  // convert string to wstring
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring utf16Text = converter.from_bytes(std::string(text, textLen));

  m_tokenizer.process(
      utf16Text.c_str(), utf16Text.length(),
      [this](std::unique_ptr<Token> token) { this->onEmit(std::move(token)); });
}

void Parser::parse(const wchar_t *text, size_t textLen) {
  m_tokenizer.process(text, textLen, [this](std::unique_ptr<Token> token) {
    this->onEmit(std::move(token));
  });
}

void Parser::onEmit(std::unique_ptr<Token> token) {
  std::cout << "type=" << token->type() << "\n";
}

} // namespace LibHTML
