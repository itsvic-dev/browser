#include "libhtml/parser.h"
#include "libdom.h"
#include "libdom/node.h"
#include "libhtml/exceptions.h"
#include "libhtml/tokens.h"
#include <codecvt>
#include <exception>
#include <iostream>
#include <locale>
#include <memory>
#include <string>

#define REPROCESS process(std::move(token))
#define CONVERT_TO(type, ptr)                                                  \
  std::unique_ptr<type>(reinterpret_cast<type *>(ptr.release()))

#define currentNode (m_nodeStack.back())

namespace LibHTML {

Parser::Parser() : document(std::make_shared<LibDOM::Document>()) {}

void Parser::parse(const char *text, size_t textLen) {
  // convert string to wstring
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring utf16Text = converter.from_bytes(std::string(text, textLen));

  m_tokenizer.process(utf16Text.c_str(), utf16Text.length(),
                      [this](std::unique_ptr<Token> token) {
                        this->process(std::move(token));
                      });
}

void Parser::parse(const wchar_t *text, size_t textLen) {
  m_tokenizer.process(text, textLen, [this](std::unique_ptr<Token> token) {
    this->process(std::move(token));
  });
}

void Parser::initialInsertion(std::unique_ptr<Token> token) {
  // https://html.spec.whatwg.org/multipage/parsing.html#the-initial-insertion-mode
  if (token->type() == CHARACTER) {
    auto charToken = CONVERT_TO(CharacterToken, token);
    if (isspace(charToken->character())) {
      return;
    }
    token = std::move(charToken);
  }

  if (token->type() == CHARACTER) {
    throw StringException("FIXME: insert a comment into the document");
    return;
  }

  if (token->type() == DOCTYPE_TOKEN) {
    auto docToken = CONVERT_TO(DoctypeToken, token);
    auto docType = std::make_shared<LibDOM::DocumentType>();
    docType->name = docToken->name();
    document->appendChild(docType);
    if (!document->parserCannotChangeMode &&
        (docToken->forceQuirks() || docToken->name() != L"html")) {
      document->mode = "quirks";
    }
    m_insertionMode = BEFORE_HTML;
    return;
  }

  if (!document->parserCannotChangeMode) {
    document->mode = "quirks";
  }
  m_insertionMode = BEFORE_HTML;
  REPROCESS;
}

#define MODE(mode, func)                                                       \
  case mode:                                                                   \
    func(std::move(token));                                                    \
    break;

void Parser::process(std::unique_ptr<Token> token) {
  std::cout << "emitted token type=" << token->type()
            << ", mode=" << m_insertionMode << "\n";
  switch (m_insertionMode) {
    MODE(INITIAL, initialInsertion)
  default:
    std::clog << "unknown insertion mode encountered: " << m_insertionMode
              << "\n";
    throw StringException("unknown insertion mode encountered");
    break;
  }
}

#undef MODE

void Parser::resetInsertionModeAppropriately() {
  // https://html.spec.whatwg.org/multipage/parsing.html#reset-the-insertion-mode-appropriately
  throw StringException("TODO: Parser::resetInsertionModeAppropriately");
}

} // namespace LibHTML
