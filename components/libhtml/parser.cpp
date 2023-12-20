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

#define CURRENT_NODE (m_nodeStack.back())

#define INSERT_HTML_ELEMENT(token)                                             \
  insertForeignElement((token).get(), NS_HTML, false)

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
  auto elem = createElement(L"html", NS_HTML);
  document->appendChild(elem);
  m_nodeStack.push_back(elem);
  m_insertionMode = BEFORE_HEAD;
  REPROCESS;
}

/** https://html.spec.whatwg.org/multipage/parsing.html#the-before-head-insertion-mode */
void Parser::beforeHead(std::unique_ptr<Token> token) {
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

  if (token->type() == DOCTYPE_TOKEN)
    return;

  if (token->type() == START_TAG) {
    auto tagToken = CONVERT_TO(TagToken, token);
    if (tagToken->name() == L"html") {
      throw StringException("TODO: Process the token using the rules for the "
                            "\"in body\" insertion mode.");
    }

    if (tagToken->name() == L"head") {
      auto elem = INSERT_HTML_ELEMENT(tagToken);
      m_headElementPointer = elem;
      m_insertionMode = IN_HEAD;
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
  auto tagToken = std::make_shared<TagToken>(START_TAG);
  tagToken->appendName(L"head");
  auto elem = INSERT_HTML_ELEMENT(tagToken);
  m_headElementPointer = elem;
  m_insertionMode = IN_HEAD;
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
    MODE(BEFORE_HEAD, beforeHead)
  case 3:
    break;
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

#define LOCAL_DEF(ln, type)                                                    \
  if (localName == ln)                                                         \
    elem = std::make_shared<type>();

/** https://dom.spec.whatwg.org/#concept-create-element */
std::shared_ptr<LibDOM::HTMLElement>
Parser::createElement(std::wstring localName, std::wstring ns,
                      std::wstring prefix) {
  std::shared_ptr<LibDOM::HTMLElement> elem = nullptr;

  LOCAL_DEF(L"html", LibDOM::HTMLHtmlElement)
  LOCAL_DEF(L"head", LibDOM::HTMLHeadElement)
  if (elem == nullptr) {
    std::wclog << L"WARNING: using default HTMLElement for localName "
               << localName << L"\n";
    elem = std::make_shared<LibDOM::HTMLElement>();
  }

  elem->namespaceURI = ns;
  elem->prefix = prefix;
  elem->localName = localName;
  elem->ownerDocument = document;
  return elem;
}

#undef LOCAL_DEF

/** https://html.spec.whatwg.org/multipage/parsing.html#create-an-element-for-the-token */
std::shared_ptr<LibDOM::Node>
Parser::createElementForToken(TagToken *token, std::wstring ns,
                              std::shared_ptr<LibDOM::Node> intendedParent) {
  (void)intendedParent;
  auto elem = createElement(token->name(), ns);
  for (const auto &attr : token->attributes) {
    auto attribute = std::make_shared<LibDOM::Attr>();
    attribute->name = attr.name;
    attribute->value = attr.value;
    elem->attributes.setNamedItem(attribute);
  }
  return elem;
}

std::shared_ptr<LibDOM::Node>
Parser::insertForeignElement(TagToken *token, std::wstring ns,
                             bool onlyAddToElementStack) {
  auto insertLocation = CURRENT_NODE;
  auto elem = createElementForToken(token, ns, insertLocation);
  if (!onlyAddToElementStack)
    insertLocation->appendChild(elem);
  m_nodeStack.push_back(elem);
  return elem;
}

} // namespace LibHTML
