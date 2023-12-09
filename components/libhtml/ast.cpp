#include "libhtml/ast.h"
#include "libhtml/dom.h"
#include "libhtml/dom/comment.h"
#include "libhtml/tokens.h"
#include <iostream>
#include <memory>

namespace LibHTML {

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
      document.appendChild(std::make_shared<DOM::Comment>(comToken->data()));
      return;
    }
    if (token->type() == DOCTYPE_TOKEN) {
      auto doctypeToken = std::static_pointer_cast<DoctypeToken>(token);
      auto documentType = std::make_shared<DOM::DocumentType>();
      documentType->name = doctypeToken->name();
      // FIXME: if the document is not an iframe srcdoc document, [...]
      if (!document.parserCannotChangeMode &&
          (doctypeToken->forceQuirks() || doctypeToken->name() != L"html")) {
        document.mode = "quirks";
      }
      document.appendChild(documentType);
      insertionMode = BEFORE_HTML;
      return;
    }
    // "If the document is not an iframe srcdoc document, then this is a parse
    // error; if the parser cannot change the mode flag is false, set the
    // Document to quirks mode.
    // In any case, switch the insertion mode to "before html", then reprocess
    // the token."
    if (!document.parserCannotChangeMode)
      document.mode = "quirks";
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
      document.appendChild(
          std::make_shared<DOM::Comment>(commentToken->data()));
      return;
    }
    if (token->type() == START_TAG) {
      auto tagToken = std::static_pointer_cast<TagToken>(token);
      if (tagToken->name() == L"html") {
        // TODO: "Create an element for the token in the HTML namespace, with
        // the Document as the intended parent. Append it to the Document
        // object. Put this element in the stack of open elements."

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
    // "Create an html element whose node document is the Document object.
    // Append it to the Document object. Put this element in the stack of open
    // elements.
    // Switch the insertion mode to "before head", then reprocess the
    // token."
    insertionMode = BEFORE_HEAD;
    tokenPtr--;
    break;
  }

  default:
    std::cerr << "[LibHTML] Unhandled insertion mode " << insertionMode << "\n";
    throw 0;
  }
}

void ASTParser::consume() { token = tokens[tokenPtr++]; }

} // namespace LibHTML
