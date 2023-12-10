#include "libhtml/ast.h"
#include "libdom.h"
#include "libdom/comment.h"
#include "libdom/element.h"
#include "libhtml/tokens.h"
#include <cassert>
#include <iostream>
#include <memory>

namespace LibHTML {

ASTParser::ASTParser() : document(std::make_shared<LibDOM::Document>()) {}

void ASTParser::parse(std::vector<std::shared_ptr<Token>> tokens) {
  this->tokens = tokens;

  while (tokenPtr < tokens.size()) {
    std::cout << "[LibHTML] token=" << tokens[tokenPtr]->type()
              << " (idx=" << tokenPtr << "), mode=" << insertionMode << "\n";
    parseTick();
  }
}

void ASTParser::parseTick() {
  consume();

  switch (insertionMode) {
  // https://html.spec.whatwg.org/multipage/parsing.html#the-initial-insertion-mode
  case INITIAL: {
    if (token->type() == CHARACTER) {
      auto charToken = std::static_pointer_cast<CharacterToken>(token);
      if (isspace(charToken->character())) {
        return; // ignore
      }
    }
    if (token->type() == COMMENT) {
      auto comToken = std::static_pointer_cast<CommentToken>(token);
      document->appendChild(
          std::make_shared<LibDOM::Comment>(comToken->data()));
      return;
    }
    if (token->type() == DOCTYPE_TOKEN) {
      auto doctypeToken = std::static_pointer_cast<DoctypeToken>(token);
      auto documentType = std::make_shared<LibDOM::DocumentType>();
      documentType->name = doctypeToken->name();
      // FIXME: if the document is not an iframe srcdoc document, [...]
      if (!document->parserCannotChangeMode &&
          (doctypeToken->forceQuirks() || doctypeToken->name() != L"html")) {
        document->mode = "quirks";
      }
      document->appendChild(documentType);
      insertionMode = BEFORE_HTML;
      return;
    }
    // "If the document is not an iframe srcdoc document, then this is a parse
    // error; if the parser cannot change the mode flag is false, set the
    // Document to quirks mode.
    // In any case, switch the insertion mode to "before html", then reprocess
    // the token."
    if (!document->parserCannotChangeMode)
      document->mode = "quirks";
    tokenPtr--;
    insertionMode = BEFORE_HTML;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#the-before-html-insertion-mode
  case BEFORE_HTML: {
    consume();
    if (token->type() == DOCTYPE_TOKEN) {
      // "Parse error. Ignore the token."
      return;
    }
    if (token->type() == COMMENT) {
      auto commentToken = std::static_pointer_cast<CommentToken>(token);
      document->appendChild(
          std::make_shared<LibDOM::Comment>(commentToken->data()));
      return;
    }
    if (token->type() == CHARACTER) {
      auto charToken = std::static_pointer_cast<CharacterToken>(token);
      if (isspace(charToken->character())) {
        return; // ignore
      }
    }
    if (token->type() == START_TAG) {
      auto tagToken = std::static_pointer_cast<TagToken>(token);
      if (tagToken->name() == L"html") {
        auto htmlElement = createElementForToken(tagToken);
        openElementStack.push_back(htmlElement);
        document->appendChild(htmlElement);
        insertionMode = BEFORE_HEAD;
        return;
      }
    }
    if (token->type() == END_TAG) {
      auto tagToken = std::static_pointer_cast<TagToken>(token);
      if (tagToken->name() != L"head" && tagToken->name() != L"body" &&
          tagToken->name() != L"html" && tagToken->name() != L"br") {
        // "Parse error. Ignore the token."
        return;
      }
    }
    auto htmlElement = createElement(document, L"html", NS_HTML);
    openElementStack.push_back(htmlElement);
    document->appendChild(htmlElement);
    insertionMode = BEFORE_HEAD;
    tokenPtr--;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#the-before-head-insertion-mode
  case BEFORE_HEAD: {
    if (token->type() == CHARACTER) {
      auto charToken = std::static_pointer_cast<CharacterToken>(token);
      if (isspace(charToken->character())) {
        return; // ignore
      }
    }
    if (token->type() == COMMENT) {
      auto commToken = std::static_pointer_cast<CommentToken>(token);
      auto comment = std::make_shared<LibDOM::Comment>(commToken->data());
      comment->ownerDocument = document;
      openElementStack.back()->appendChild(comment);
      return;
    }
    if (token->type() == DOCTYPE_TOKEN) {
      // Parse error. Ignore the token.
      return;
    }
    if (token->type() == START_TAG) {
      auto tagToken = std::static_pointer_cast<TagToken>(token);
      if (tagToken->name() == L"html") {
        // FIXME: Process the token using the rules for the "in body" insertion
        // mode.
        return;
      }
      if (tagToken->name() == L"head") {
        auto head = createElementForToken(tagToken);
        openElementStack.back()->appendChild(head);
        headElementPtr = head;
        insertionMode = IN_HEAD;
        return;
      }
    }
    if (token->type() == END_TAG) {
      auto tagToken = std::static_pointer_cast<TagToken>(token);
      if (tagToken->name() != L"head" && tagToken->name() != L"body" &&
          tagToken->name() != L"html" && tagToken->name() != L"br") {
        // "Parse error. Ignore the token."
        return;
      }
    }
    auto head = createElement(document, L"head", NS_HTML);
    openElementStack.back()->appendChild(head);
    headElementPtr = head;
    insertionMode = IN_HEAD;
    tokenPtr--;
    break;
  }

  default:
    std::cerr << "[LibHTML] Unhandled insertion mode " << insertionMode << "\n";
    throw 0;
  }
}

std::shared_ptr<LibDOM::HTMLElement>
ASTParser::createElement(std::shared_ptr<LibDOM::Document> document,
                         std::wstring ns, std::wstring localName,
                         std::wstring prefix) {
  // FIXME: handle custom elements once we bring in JS
  std::shared_ptr<LibDOM::HTMLElement> result = nullptr;
  if (localName == L"html") {
    result = std::make_shared<LibDOM::HTMLHtmlElement>();
  }
  if (localName == L"head") {
    result = std::make_shared<LibDOM::HTMLHeadElement>();
  }

  assert(result != nullptr);

  result->ownerDocument = document;
  result->namespaceURI = ns;
  result->localName = localName;
  result->tagName = localName;
  result->prefix = prefix;

  return result;
}

std::shared_ptr<LibDOM::HTMLElement>
ASTParser::createElementForToken(std::shared_ptr<TagToken> token) {
  auto document = this->document;
  auto localName = token->name();
  auto element = createElement(document, NS_HTML, localName);
  // FIXME: attributes
  return element;
}

void ASTParser::consume() { token = tokens[tokenPtr++]; }

} // namespace LibHTML
