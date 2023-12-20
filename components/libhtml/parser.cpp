#include "libhtml/parser.h"
#include "libdom.h"
#include "libdom/element.h"
#include "libdom/node.h"
#include "libhtml/exceptions.h"
#include "libhtml/tokens.h"
#include <codecvt>
#include <exception>
#include <iostream>
#include <locale>
#include <memory>
#include <string>
#include <utility>

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

/** https://html.spec.whatwg.org/multipage/parsing.html#the-initial-insertion-mode */
void Parser::initialInsertion(std::unique_ptr<Token> token) {
  if (token->type() == CHARACTER) {
    auto charToken = CONVERT_TO(CharacterToken, token);
    if (isspace(charToken->character())) {
      return;
    }
    token = std::move(charToken);
  }

  if (token->type() == COMMENT) {
    insertComment(std::move(token), document);
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

/** https://html.spec.whatwg.org/multipage/parsing.html#the-before-html-insertion-mode */
void Parser::beforeHtml(std::unique_ptr<Token> token) {
  if (token->type() == DOCTYPE_TOKEN)
    return;

  if (token->type() == COMMENT) {
    insertComment(std::move(token), document);
    return;
  }

  if (token->type() == CHARACTER) {
    auto charToken = CONVERT_TO(CharacterToken, token);
    if (isspace(charToken->character())) {
      return;
    }
    token = std::move(charToken);
  }

  if (token->type() == START_TAG) {
    auto tagToken = CONVERT_TO(TagToken, token);
    if (tagToken->name() == L"html") {
      auto elem = createElementForToken(tagToken.get(), NS_HTML, document);
      document->appendChild(elem);
      m_nodeStack.push_back(elem);
      m_insertionMode = BEFORE_HEAD;
      return;
    }
    token = std::move(tagToken);
  }

  if (token->type() == END_TAG) {
    auto tagToken = CONVERT_TO(TagToken, token);
    if (tagToken->name() == L"html" || tagToken->name() == L"head" ||
        tagToken->name() == L"body" || tagToken->name() == L"br") {
      token = std::move(tagToken);
      goto anythingElse;
    }
    return;
  }

anythingElse:
  auto elem = std::make_shared<LibDOM::HTMLHtmlElement>();
  elem->nodeName = L"html";
  elem->localName = L"html";
  elem->namespaceURI = NS_HTML;
  elem->ownerDocument = document;
  document->appendChild(elem);
  m_nodeStack.push_back(elem);
  m_insertionMode = BEFORE_HEAD;
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
    MODE(BEFORE_HTML, beforeHtml)
  default:
    std::clog << "unknown insertion mode encountered: " << m_insertionMode
              << "\n";
    throw StringException("unknown insertion mode encountered");
    break;
  }
}

#undef MODE

/** https://html.spec.whatwg.org/multipage/parsing.html#reset-the-insertion-mode-appropriately */
void Parser::resetInsertionModeAppropriately() {
  throw StringException("TODO: Parser::resetInsertionModeAppropriately");
}

/** https://html.spec.whatwg.org/multipage/parsing.html#insert-a-comment */
void Parser::insertComment(std::unique_ptr<Token> token,
                           std::shared_ptr<LibDOM::Node> position) {
  (void)token;
  (void)position;
  throw StringException("TODO: Parser::insertComment");
}

/** https://html.spec.whatwg.org/multipage/parsing.html#create-an-element-for-the-token */
std::shared_ptr<LibDOM::Node>
Parser::createElementForToken(TagToken *token, std::wstring ns,
                              std::shared_ptr<LibDOM::Node> intendedParent) {
  (void)token;
  (void)ns;
  (void)intendedParent;
  throw StringException("TODO: Parser::createElementForToken");
}

} // namespace LibHTML
