#include "libhtml/parser.h"
#include "libdom.h"
#include "libdom/element.h"
#include "libdom/node.h"
#include "libdom/text.h"
#include "libhtml/exceptions.h"
#include "libhtml/tokenizer.h"
#include "libhtml/tokens.h"
#include <algorithm>
#include <cassert>
#include <codecvt>
#include <cwchar>
#include <exception>
#include <iostream>
#include <locale>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define REPROCESS process(std::move(token))
#define CONVERT_TO(type, ptr)                                                  \
  std::unique_ptr<type>(reinterpret_cast<type *>(ptr.release()))

#define CURRENT_NODE (m_nodeStack.back())

#define INSERT_HTML_ELEMENT(token)                                             \
  insertForeignElement((token).get(), NS_HTML, false)

#define IS_ONE_OF(name, names)                                                 \
  (std::find_if(names.begin(), names.end(), [name](std::wstring otherName) {   \
     return otherName == name;                                                 \
   }) != names.end())

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
  try {
    m_tokenizer.process(text, textLen, [this](std::unique_ptr<Token> token) {
      if (!m_isParsing)
        throw 0;
      this->process(std::move(token));
    });
  } catch (int &a) {
    return;
  }
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
      inBody(std::move(tagToken));
      return;
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
      insertCharacter(std::move(charToken));
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
      inBody(std::move(tagToken));
      return;
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

/** https://html.spec.whatwg.org/multipage/parsing.html#the-after-head-insertion-mode */
void Parser::afterHead(std::unique_ptr<Token> token) {
  if (token->type() == CHARACTER) {
    auto charToken = CONVERT_TO(CharacterToken, token);
    if (isspace(charToken->character())) {
      insertCharacter(std::move(charToken));
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
      inBody(std::move(tagToken));
      return;
    }
    if (tagToken->name() == L"body") {
      INSERT_HTML_ELEMENT(tagToken);
      m_framesetOk = false;
      m_insertionMode = IN_BODY;
      return;
    }
    if (tagToken->name() == L"frameset") {
      INSERT_HTML_ELEMENT(tagToken);
      m_insertionMode = IN_FRAMESET;
      return;
    }
    const std::vector<std::wstring> names = {
        L"base",     L"basefont", L"bgsound", L"link",     L"meta",
        L"noframes", L"script",   L"style",   L"template", L"title"};
    auto name = tagToken->name();
    if (IS_ONE_OF(name, names)) {
      throw StringException("TODO: Process the token using the rules for the "
                            "\"in head\" insertion mode.");
    }
    if (name == L"head")
      return;
    token = std::move(tagToken);
  }

  if (token->type() == END_TAG) {
    auto tagToken = CONVERT_TO(TagToken, token);
    if (tagToken->name() == L"body" || tagToken->name() == L"html" ||
        tagToken->name() == L"br") {
      token = std::move(tagToken);
      goto anythingElse;
    }
    return;
  }

anythingElse:
  auto tagToken = std::unique_ptr<TagToken>(new TagToken(START_TAG));
  tagToken->appendName(L"body");
  INSERT_HTML_ELEMENT(tagToken);
  m_insertionMode = IN_BODY;
  REPROCESS;
}

/** https://html.spec.whatwg.org/multipage/parsing.html#parsing-main-inbody */
void Parser::inBody(std::unique_ptr<Token> token) {
  if (token->type() == CHARACTER) {
    auto charToken = CONVERT_TO(CharacterToken, token);
    if (charToken->character() == 0) {
      return;
    }
    if (isspace(charToken->character())) {
      reconstructActiveFormattingElements();
      insertCharacter(std::move(charToken));
      return;
    }
    reconstructActiveFormattingElements();
    insertCharacter(std::move(charToken));
    m_framesetOk = false;
    return;
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
      for (const auto &attr : tagToken->attributes) {
        auto htmlElem = std::static_pointer_cast<LibDOM::HTMLHtmlElement>(
            *m_nodeStack.begin());
        if (htmlElem->hasAttribute(attr.name))
          continue;
        htmlElem->setAttribute(attr.name, attr.value);
      }
      return;
    }

    const std::vector<std::wstring> names = {
        L"base",     L"basefont", L"bgsound", L"link",     L"meta",
        L"noframes", L"script",   L"style",   L"template", L"title"};
    auto name = tagToken->name();
    if (IS_ONE_OF(name, names)) {
      inHead(std::move(tagToken));
      return;
    }

    const std::vector<std::wstring> names2 = {
        L"address",  L"article",    L"aside",   L"blockquote", L"center",
        L"details",  L"dialog",     L"dir",     L"div",        L"dl",
        L"fieldset", L"figcaption", L"figure",  L"footer",     L"header",
        L"hgroup",   L"main",       L"menu",    L"nav",        L"ol",
        L"p",        L"search",     L"section", L"summary",    L"ul"};
    if (IS_ONE_OF(name, names2)) {
      if (stackHasInButtonScope(L"p"))
        closePElem();
      INSERT_HTML_ELEMENT(tagToken);
      return;
    }

    const std::vector<std::wstring> names3 = {L"h1", L"h2", L"h3",
                                              L"h4", L"h5", L"h6"};
    if (IS_ONE_OF(name, names3)) {
      if (stackHasInButtonScope(L"p"))
        closePElem();
      auto currentNodeName = CURRENT_NODE->nodeName;
      if (IS_ONE_OF(currentNodeName, names3)) {
        m_nodeStack.pop_back();
      }
      INSERT_HTML_ELEMENT(tagToken);
      return;
    }

    if (name == L"pre" || name == L"listing") {
      if (stackHasInButtonScope(L"p"))
        closePElem();

      INSERT_HTML_ELEMENT(tagToken);
      // FIXME: If the next token is a U+000A LINE FEED (LF) character token,
      // then ignore that token and move on to the next one.
      m_framesetOk = false;
      return;
    }

    if (name == L"form") {
      if (m_formElementPointer != nullptr)
        return;
      if (stackHasInButtonScope(L"p"))
        closePElem();
      auto elem = INSERT_HTML_ELEMENT(tagToken);
      m_formElementPointer = elem;
      return;
    }

    if (name == L"li") {
      m_framesetOk = false;
      throw StringException("todo: in body: li tag (too lazy)");
    }

    if (name == L"dd" || name == L"dt") {
      m_framesetOk = false;
      throw StringException("todo: in body: dd/dt tag (too lazy)");
    }

    if (name == L"plaintext") {
      if (stackHasInButtonScope(L"p"))
        closePElem();
      INSERT_HTML_ELEMENT(tagToken);
      m_tokenizer.currentState = PLAINTEXT;
      return;
    }

    if (name == L"button") {
      for (auto elem : m_nodeStack) {
        if (elem->nodeName == L"button" && isInScope(elem)) {
          generateImpliedEndTags();
          popStackUntil(L"button");
          break;
        }
      }

      reconstructActiveFormattingElements();
      INSERT_HTML_ELEMENT(tagToken);
      m_framesetOk = false;
      return;
    }

    if (name == L"body") {
      if (m_nodeStack.size() == 1 || m_nodeStack[2]->nodeName != L"body")
        return;

      // TODO: html body element
      m_framesetOk = false;
      auto body = std::static_pointer_cast<LibDOM::HTMLElement>(m_nodeStack[2]);
      for (const auto &attr : tagToken->attributes) {
        if (body->hasAttribute(attr.name))
          continue;
        body->setAttribute(attr.name, attr.value);
      }
      return;
    }

    if (name == L"frameset") {
      throw StringException("TODO: in body: frameset start tag");
    }

    if (name == L"a") {
      // FIXME: If the list of active formatting elements contains an a element
      // between the end of the list and the last marker on the list (or the
      // start of the list if there is no marker on the list), then this is a
      // parse error; run the adoption agency algorithm for the token, then
      // remove that element from the list of active formatting elements and the
      // stack of open elements if the adoption agency algorithm didn't already
      // remove it (it might not have if the element is not in table scope).

      reconstructActiveFormattingElements();
      auto elem = INSERT_HTML_ELEMENT(tagToken);
      pushOntoActiveFormattingElems(elem);
      return;
    }

    const std::vector<std::wstring> names4 = {
        L"b", L"big",   L"code",   L"em",     L"font", L"i",
        L"s", L"small", L"strike", L"strong", L"tt",   L"u"};
    if (IS_ONE_OF(name, names4)) {
      reconstructActiveFormattingElements();
      INSERT_HTML_ELEMENT(tagToken);
      return;
    }

    if (name == L"nobr") {
      reconstructActiveFormattingElements();
      if (stackHasInScope(L"nobr")) {
        adoptionAgency(tagToken.get());
        reconstructActiveFormattingElements();
      }
      auto elem = INSERT_HTML_ELEMENT(tagToken);
      pushOntoActiveFormattingElems(elem);
      return;
    }

    if (name == L"applet" || name == L"marquee" || name == L"object") {
      reconstructActiveFormattingElements();
      INSERT_HTML_ELEMENT(tagToken);
      // FIXME: Insert a marker at the end of the list of active formatting
      // elements.
      m_framesetOk = false;
    }

    if (name == L"table") {
      if (document->mode != "quirks" && stackHasInButtonScope(L"p"))
        closePElem();
      INSERT_HTML_ELEMENT(tagToken);
      m_framesetOk = false;
      m_insertionMode = IN_TABLE;
      return;
    }

    if (name == L"area" || name == L"br" || name == L"embed" ||
        name == L"img" || name == L"keygen" || name == L"wbr") {
      reconstructActiveFormattingElements();
      INSERT_HTML_ELEMENT(tagToken);
      m_nodeStack.pop_back();
      m_framesetOk = false;
      return;
    }

    if (name == L"input") {
      reconstructActiveFormattingElements();
      auto elem = std::static_pointer_cast<LibDOM::HTMLElement>(
          INSERT_HTML_ELEMENT(tagToken));
      m_nodeStack.pop_back();
      if (!elem->hasAttribute(L"type") ||
          wcscasecmp(elem->getAttribute(L"type").c_str(), L"hidden") != 0) {
        m_framesetOk = false;
      }
      return;
    }

    if (name == L"hr") {
      if (stackHasInButtonScope(L"p"))
        closePElem();
      INSERT_HTML_ELEMENT(tagToken);
      m_nodeStack.pop_back();
      m_framesetOk = false;
      return;
    }

    if (name == L"image") {
      tagToken->setName(L"img");
      process(std::move(tagToken));
      return;
    }

    if (name == L"textarea") {
      INSERT_HTML_ELEMENT(tagToken);
      // FIXME: If the next token is a U+000A LINE FEED (LF) character token,
      // then ignore that token and move on to the next one. (Newlines at the
      // start of textarea elements are ignored as an authoring convenience.)
      m_tokenizer.currentState = RCDATA;
      m_originalInsertionMode = m_insertionMode;
      m_framesetOk = false;
      m_insertionMode = TEXT;
      return;
    }

    if (name == L"xmp") {
      if (stackHasInButtonScope(L"p"))
        closePElem();
      reconstructActiveFormattingElements();
      m_framesetOk = false;
      genericRawTextParse(std::move(tagToken));
      return;
    }

    if (name == L"iframe") {
      m_framesetOk = false;
      genericRawTextParse(std::move(tagToken));
      return;
    }

    if (name == L"noembed" || (name == L"noscript" && m_scriptingFlag)) {
      genericRawTextParse(std::move(tagToken));
      return;
    }

    if (name == L"select") {
      reconstructActiveFormattingElements();
      INSERT_HTML_ELEMENT(tagToken);
      m_framesetOk = false;
      if (m_insertionMode == IN_TABLE || m_insertionMode == IN_CAPTION ||
          m_insertionMode == IN_TABLE_BODY || m_insertionMode == IN_ROW ||
          m_insertionMode == IN_CELL)
        m_insertionMode = IN_SELECT_IN_TABLE;
      else
        m_insertionMode = IN_SELECT;
      return;
    }

    if (name == L"optgroup" || name == L"option") {
      if (CURRENT_NODE->nodeName == L"option")
        m_nodeStack.pop_back();
      reconstructActiveFormattingElements();
      INSERT_HTML_ELEMENT(tagToken);
      return;
    }

    if (name == L"rb" || name == L"rtc") {
      if (stackHasInScope(L"ruby"))
        generateImpliedEndTags();
      INSERT_HTML_ELEMENT(tagToken);
      return;
    }

    if (name == L"rp" || name == L"rt") {
      if (stackHasInScope(L"ruby"))
        generateImpliedEndTagsExceptFor(L"rtc");
      INSERT_HTML_ELEMENT(tagToken);
      return;
    }

    if (name == L"math") {
      throw StringException("TODO: in body: MathML support");
    }

    if (name == L"SVG") {
      throw StringException("TODO: in body: SVG support");
    }

    const std::vector<std::wstring> names5 = {
        L"caption", L"col",   L"colgroup", L"frame", L"head", L"tbody",
        L"td",      L"tfoot", L"th",       L"thead", L"tr"};
    if (IS_ONE_OF(name, names5)) {
      return;
    }

    reconstructActiveFormattingElements();
    INSERT_HTML_ELEMENT(tagToken);
  }

  if (token->type() == END_OF_FILE) {
    stopParsing();
    return;
  }

  if (token->type() == END_TAG) {
    auto tagToken = CONVERT_TO(TagToken, token);
    if (tagToken->name() == L"body") {
      if (std::find_if(m_nodeStack.begin(), m_nodeStack.end(),
                       [](std::shared_ptr<LibDOM::Node> node) {
                         return node->nodeName == L"body";
                       }) == m_nodeStack.end()) {
        return;
      }
      m_insertionMode = AFTER_BODY;
      return;
    }

    if (tagToken->name() == L"html") {
      if (std::find_if(m_nodeStack.begin(), m_nodeStack.end(),
                       [](std::shared_ptr<LibDOM::Node> node) {
                         return node->nodeName == L"body";
                       }) == m_nodeStack.end()) {
        return;
      }
      m_insertionMode = AFTER_BODY;
      token = std::move(tagToken);
      REPROCESS;
      return;
    }

    auto name = tagToken->name();
    const std::vector<std::wstring> names = {
        L"address", L"article",  L"aside",      L"blockquote", L"button",
        L"center",  L"details",  L"dialog",     L"dir",        L"div",
        L"dl",      L"fieldset", L"figcaption", L"figure",     L"footer",
        L"header",  L"hgroup",   L"listing",    L"main",       L"menu",
        L"nav",     L"ol",       L"pre",        L"search",     L"section",
        L"summary", L"ul",
    };
    if (IS_ONE_OF(name, names)) {
      if (!stackHasInScope(name))
        return;
      generateImpliedEndTags();
      popStackUntil(name);
      return;
    }

    if (name == L"form") {
      auto node = m_formElementPointer;
      m_formElementPointer = nullptr;
      if (node == nullptr
          /* FIXME: or node stack does not have <node> in scope */)
        return;
      generateImpliedEndTags();
      m_nodeStack.erase(
          std::find(m_nodeStack.begin(), m_nodeStack.end(), node));
      return;
    }

    if (name == L"p") {
      if (!stackHasInButtonScope(L"p")) {
        auto newTagToken = std::make_shared<TagToken>(START_TAG);
        newTagToken->appendName(L"p");
        INSERT_HTML_ELEMENT(newTagToken);
      }
      closePElem();
      return;
    }

    if (name == L"li") {
      // FIXME: If the stack of open elements does not have an li element in
      // list item scope, then this is a parse error; ignore the token.
      generateImpliedEndTagsExceptFor(L"li");
      throw StringException("TODO: in body: li end tag (too lazy)");
    }

    if (name == L"dd" || name == L"dt") {
      if (!stackHasInScope(name))
        return;
      generateImpliedEndTagsExceptFor(name);
      throw StringException("TODO: in body: dd/dt end tags (too lazy)");
    }

    const std::vector<std::wstring> names2 = {L"h1", L"h2", L"h3",
                                              L"h4", L"h5", L"h6"};
    if (IS_ONE_OF(name, names2)) {
      if (!stackHasInScope(name))
        return;
      generateImpliedEndTags();
      while (true) {
        auto name = CURRENT_NODE->nodeName;
        m_nodeStack.pop_back();
        if (IS_ONE_OF(name, names2))
          break;
      }
      return;
    }

    const std::vector<std::wstring> names4 = {
        L"a",    L"b", L"big",   L"code",   L"em",     L"font", L"i",
        L"nobr", L"s", L"small", L"strike", L"strong", L"tt",   L"u",
    };
    if (IS_ONE_OF(name, names4)) {
      adoptionAgency(tagToken.get());
      return;
    }

    if (name == L"applet" || name == L"marquee" || name == L"object") {
      if (!stackHasInScope(name))
        return;
      generateImpliedEndTags();
      throw StringException("TODO: in body: marquee end tag (too lazy)");
    }

    auto node = CURRENT_NODE;
  loop:
    if (node->nodeName == tagToken->name()) {
      generateImpliedEndTagsExceptFor(tagToken->name());
      while (CURRENT_NODE != node)
        m_nodeStack.pop_back();
      m_nodeStack.pop_back();
      return;
    }
    // FIXME: Otherwise, if node is in the special category, then this is a
    // parse error; ignore the token, and return.
    node = m_nodeStack.end()[-2];
    goto loop;
  }

  assert(!"unreachable");
}

/** https://html.spec.whatwg.org/multipage/parsing.html#parsing-main-incdata
 */
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
  assert(token != nullptr);
  std::cout << "emitted token type=" << token->type()
            << ", mode=" << m_insertionMode << "\n";
  switch (m_insertionMode) {
    MODE(INITIAL, initialInsertion)
    MODE(BEFORE_HTML, beforeHtml)
    MODE(BEFORE_HEAD, beforeHead)
    MODE(IN_HEAD, inHead)
    MODE(AFTER_HEAD, afterHead)
    MODE(IN_BODY, inBody)
    MODE(TEXT, text)
    case AFTER_BODY:
      // FIXME: implement
      break;
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
  // TODO: Parser::resetInsertionModeAppropriately
  throw StringException("TODO: Parser::resetInsertionModeAppropriately");
}

/** https://html.spec.whatwg.org/multipage/parsing.html#reconstruct-the-active-formatting-elements */
void Parser::reconstructActiveFormattingElements() {
  if (m_activeFormattingElems.empty())
    return;
  // FIXME: markers
  if (std::find(m_nodeStack.begin(), m_nodeStack.end(),
                m_activeFormattingElems.back()) != m_nodeStack.end()) {
    return;
  }

  auto entry = m_activeFormattingElems.back();
rewind:
  if (entry == m_activeFormattingElems.begin()[0])
    goto create;
  entry = std::find(m_activeFormattingElems.begin(),
                    m_activeFormattingElems.end(), entry)[-1];
  if (std::find(m_nodeStack.begin(), m_nodeStack.end(), entry) !=
      m_nodeStack.end())
    goto rewind;

advance:
  entry = std::find(m_activeFormattingElems.begin(),
                    m_activeFormattingElems.end(), entry)[1];
create:
  auto tagToken = std::make_shared<TagToken>(START_TAG);
  tagToken->setName(entry->localName);
  auto newElem = INSERT_HTML_ELEMENT(tagToken);
  auto it = std::find(m_activeFormattingElems.begin(),
                      m_activeFormattingElems.end(), entry);
  *it = newElem;
  if (newElem != m_activeFormattingElems.back())
    goto advance;
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
  elem->nodeName = localName;
  elem->ownerDocument = document;
  return elem;
}

#undef LOCAL_DEF

/** https://html.spec.whatwg.org/multipage/parsing.html#create-an-element-for-the-token */
std::shared_ptr<LibDOM::Element>
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

std::shared_ptr<LibDOM::Element>
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

/** https://html.spec.whatwg.org/multipage/parsing.html#stop-parsing */
void Parser::stopParsing() {
  m_isParsing = false;
  m_insertionMode = UNDEFINED_MODE;
  // update document readiness to interactive
  m_nodeStack.clear();
  // do script stuff
}

/** https://html.spec.whatwg.org/multipage/parsing.html#generate-implied-end-tags */
void Parser::generateImpliedEndTags() {
  const std::vector<std::wstring> endTags = {
      L"dd", L"dt", L"li", L"optgroup", L"option",
      L"p",  L"rb", L"rp", L"rt",       L"rtc"};
  while (true) {
    auto name = CURRENT_NODE->nodeName;
    if (std::find(endTags.begin(), endTags.end(), name) == endTags.end())
      return;
    m_nodeStack.pop_back();
  }
}

void Parser::generateImpliedEndTagsExceptFor(std::wstring tagName) {
  (void)tagName;
  // TODO: Parser::generateImpliedEndTagsExceptFor
  const std::vector<std::wstring> endTags = {
      L"dd", L"dt", L"li", L"optgroup", L"option",
      L"p",  L"rb", L"rp", L"rt",       L"rtc"};
  while (true) {
    auto name = CURRENT_NODE->nodeName;
    auto it = std::find_if(endTags.begin(), endTags.end(),
                           [tagName, name](std::wstring otherName) {
                             return tagName != otherName && name == otherName;
                           });
    if (it == endTags.end())
      return;
    m_nodeStack.pop_back();
  }
}

/** https://html.spec.whatwg.org/multipage/parsing.html#close-a-p-element */
void Parser::closePElem() {
  generateImpliedEndTagsExceptFor(L"p");
  popStackUntil(L"p");
}

/** https://html.spec.whatwg.org/multipage/parsing.html#push-onto-the-list-of-active-formatting-elements */
void Parser::pushOntoActiveFormattingElems(
    std::shared_ptr<LibDOM::Element> node) {
  (void)node;
  // FIXME: markers
  int count = 0;
  auto it = m_activeFormattingElems.end();
  for (const auto &item : m_activeFormattingElems) {
    if (item->nodeName == node->nodeName &&
        item->namespaceURI == node->namespaceURI
        /* FIXME: attribute check */) {
      count++;
      if (count == 3) {
        it = std::find(m_activeFormattingElems.begin(),
                       m_activeFormattingElems.end(), item);
        break;
      }
    }
  }

  if (it != m_activeFormattingElems.end()) {
    m_activeFormattingElems.erase(it);
  }

  m_activeFormattingElems.push_back(node);
}

/** https://html.spec.whatwg.org/multipage/parsing.html#has-an-element-in-scope
 */
bool Parser::isInScope(std::shared_ptr<LibDOM::Element> targetNode) {
  auto node = CURRENT_NODE;
  const std::vector<std::wstring> scope = {
      L"applet", L"caption", L"html",   L"table",    L"td",
      L"th",     L"marquee", L"object", L"template",
  };
  // TODO: add MathML and SVG scopes
step2:
  if (node == targetNode)
    return true;
  auto name = node->nodeName;
  if (IS_ONE_OF(name, scope))
    return false;
  node = std::find(m_nodeStack.begin(), m_nodeStack.end(), node)[-1];
  goto step2;
}

bool Parser::isInButtonScope(std::shared_ptr<LibDOM::Element> targetNode) {
  auto node = CURRENT_NODE;
  const std::vector<std::wstring> scope = {
      L"applet", L"caption", L"html",   L"table",    L"td",
      L"th",     L"marquee", L"object", L"template", L"button",
  };
  // TODO: add MathML and SVG scopes
step2:
  if (node == targetNode)
    return true;
  auto name = node->nodeName;
  if (IS_ONE_OF(name, scope))
    return false;
  node = std::find(m_nodeStack.begin(), m_nodeStack.end(), node)[-1];
  goto step2;
}

bool Parser::stackHasInScope(std::wstring tagName) {
  for (auto elem : m_nodeStack) {
    if (elem->nodeName == tagName && isInScope(elem)) {
      return true;
    }
  }
  return false;
}

bool Parser::stackHasInButtonScope(std::wstring tagName) {
  for (auto elem : m_nodeStack) {
    if (elem->nodeName == tagName && isInButtonScope(elem)) {
      return true;
    }
  }
  return false;
}

/** https://html.spec.whatwg.org/multipage/parsing.html#adoption-agency-algorithm
 */
void Parser::adoptionAgency(TagToken *token) {
  (void)token;
  // TODO: Parser::adoptionAgency
  throw StringException("TODO: Parser::adoptionAgency");
}

void Parser::popStackUntil(std::wstring tagName) {
  // check if tag is in stack before going nuclear
  if (std::find_if(m_nodeStack.begin(), m_nodeStack.end(),
                   [tagName](std::shared_ptr<LibDOM::Node> node) {
                     return node->nodeName == tagName;
                   }) == m_nodeStack.end()) {
    std::wclog << "WARNING: attempted to pop '" << tagName
               << "' from the stack but it's not there" << std::endl;
    std::wclog << "stack view:\n";
    for (const auto &item : m_nodeStack) {
      std::wclog << " - " << item->internalName().c_str() << " "
                 << item->nodeName << "\n";
    }
    return;
  }
  while (true) {
    auto name = CURRENT_NODE->nodeName;
    // if the current node is the element to pop until, pop it and return
    if (name == tagName) {
      m_nodeStack.pop_back();
      return;
    }
    // otherwise, pop and loop
    m_nodeStack.pop_back();
  }
}

} // namespace LibHTML
