#include "libhtml/parser.h"
#include "libdom.h"
#include "libdom/element.h"
#include "libdom/node.h"
#include "libdom/text.h"
#include "libhtml/exceptions.h"
#include "libhtml/tokenizer.h"
#include "libhtml/tokens.h"
#include <cassert>
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
    insertComment(std::move(token), CURRENT_NODE);
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

/** https://html.spec.whatwg.org/multipage/parsing.html#parsing-main-inhead */
void Parser::inHead(std::unique_ptr<Token> token) {
  if (token->type() == CHARACTER) {
    auto charToken = CONVERT_TO(CharacterToken, token);
    if (isspace(charToken->character())) {
      return;
    }
    token = std::move(charToken);
  }

  if (token->type() == COMMENT) {
    insertComment(std::move(token), CURRENT_NODE);
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

    if (tagToken->name() == L"template") {
      throw StringException("TODO: in head: template end tag");
    }

    if (tagToken->name() == L"base" || tagToken->name() == L"basefont" ||
        tagToken->name() == L"bgsound" || tagToken->name() == L"link") {
      auto elem = INSERT_HTML_ELEMENT(tagToken);
      m_nodeStack.pop_back();
      return;
    }

    if (tagToken->name() == L"meta") {
      auto elem = INSERT_HTML_ELEMENT(tagToken);
      m_nodeStack.pop_back();
      // FIXME: proper charset/content-type encoding handling
      return;
    }

    if (tagToken->name() == L"title") {
      genericRcdataParse(std::move(tagToken));
      return;
    }

    if (tagToken->name() == L"noframes" || tagToken->name() == L"style" ||
        (tagToken->name() == L"noscript" && m_scriptingFlag)) {
      genericRawTextParse(std::move(tagToken));
      return;
    }

    if (tagToken->name() == L"noscript" && !m_scriptingFlag) {
      INSERT_HTML_ELEMENT(tagToken);
      m_insertionMode = IN_HEAD_NOSCRIPT;
      return;
    }

    if (tagToken->name() == L"script") {
      auto location = CURRENT_NODE;
      auto elem = createElementForToken(tagToken.get(), NS_HTML, location);
      // FIXME: Set the element's parser document to the Document, and set the
      // element's force async to false.
      location->appendChild(elem);
      m_nodeStack.push_back(elem);
      m_tokenizer.currentState = SCRIPT_DATA;
      m_originalInsertionMode = m_insertionMode;
      m_insertionMode = TEXT;
      return;
    }

    if (tagToken->name() == L"head") {
      return;
    }
    token = std::move(tagToken);
  }

  if (token->type() == END_TAG) {
    auto tagToken = CONVERT_TO(TagToken, token);
    if (tagToken->name() == L"head") {
      m_nodeStack.pop_back();
      m_insertionMode = AFTER_HEAD;
      return;
    }

    if (tagToken->name() == L"template") {
      throw StringException("TODO: in head: template end tag");
    }

    if (tagToken->name() == L"body" || tagToken->name() == L"html" ||
        tagToken->name() == L"br") {
      token = std::move(tagToken);
      goto anythingElse;
    }
    return;
  }

anythingElse:
  m_nodeStack.pop_back();
  m_insertionMode = AFTER_HEAD;
  REPROCESS;
}

/** https://html.spec.whatwg.org/multipage/parsing.html#parsing-main-incdata */
void Parser::text(std::unique_ptr<Token> token) {
  if (token->type() == CHARACTER) {
    auto charToken = CONVERT_TO(CharacterToken, token);
    insertCharacter(std::move(charToken));
    return;
  }

  if (token->type() == END_OF_FILE) {
    m_nodeStack.pop_back();
    m_insertionMode = m_originalInsertionMode;
    REPROCESS;
    return;
  }

  if (token->type() == END_TAG) {
    auto tagToken = CONVERT_TO(TagToken, token);
    // FIXME: special script handling
    m_nodeStack.pop_back();
    m_insertionMode = m_originalInsertionMode;
    return;
  }

  assert(!"should be unreachable");
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
    MODE(IN_HEAD, inHead)
    MODE(TEXT, text)
  default:
    std::cout << "unknown insertion mode encountered: " << m_insertionMode
              << std::endl;
    throw StringException("unknown insertion mode encountered");
    break;
  }
}

#undef MODE

/** https://html.spec.whatwg.org/multipage/parsing.html#reset-the-insertion-mode-appropriately */
void Parser::resetInsertionModeAppropriately() {
  throw StringException("TODO: Parser::resetInsertionModeAppropriately");
}

/** https://html.spec.whatwg.org/multipage/parsing.html#insert-a-character */
void Parser::insertCharacter(std::unique_ptr<CharacterToken> token) {
  auto data = token->character();
  auto location = CURRENT_NODE;

  if (location->nodeType == LibDOM::Node::DOCUMENT_NODE)
    return;

  std::shared_ptr<LibDOM::Text> text;
  if (!location->childNodes.empty() &&
      location->childNodes.back()->nodeType == LibDOM::Node::TEXT_NODE) {
    text = std::static_pointer_cast<LibDOM::Text>(location->childNodes.back());
  } else {
    text = std::shared_ptr<LibDOM::Text>(new LibDOM::Text());
    location->appendChild(text);
  }

  text->data += data;
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

/** https://html.spec.whatwg.org/multipage/parsing.html#generic-raw-text-element-parsing-algorithm */
void Parser::genericRawTextParse(std::unique_ptr<TagToken> token) {
  INSERT_HTML_ELEMENT(token);
  m_tokenizer.currentState = RAWTEXT;
  m_originalInsertionMode = m_insertionMode;
  m_insertionMode = TEXT;
}

/** https://html.spec.whatwg.org/multipage/parsing.html#generic-rcdata-element-parsing-algorithm */
void Parser::genericRcdataParse(std::unique_ptr<TagToken> token) {
  INSERT_HTML_ELEMENT(token);
  m_tokenizer.currentState = RCDATA;
  m_originalInsertionMode = m_insertionMode;
  m_insertionMode = TEXT;
}

} // namespace LibHTML
