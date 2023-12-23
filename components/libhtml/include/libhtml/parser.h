#ifndef LIBHTML_PARSER_H
#define LIBHTML_PARSER_H

#include "libdom.h"
#include "libdom/element.h"
#include "libdom/node.h"
#include "libhtml/tokenizer.h"
#include "libhtml/tokens.h"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace LibHTML {

#define NS_HTML L"http://www.w3.org/1999/xhtml"

enum ParserMode {
  UNDEFINED_MODE,
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

  void reset();
  void parse(const char *text, size_t textLen);
  void parse(const wchar_t *text, size_t textLen);

  std::shared_ptr<LibDOM::Document> document;

private:
  void process(std::unique_ptr<Token> token);

  void initialInsertion(std::unique_ptr<Token> token);
  void beforeHtml(std::unique_ptr<Token> token);
  void beforeHead(std::unique_ptr<Token> token);
  void inHead(std::unique_ptr<Token> token);
  void afterHead(std::unique_ptr<Token> token);
  void inBody(std::unique_ptr<Token> token);
  void text(std::unique_ptr<Token> token);

  /** https://html.spec.whatwg.org/multipage/parsing.html#reset-the-insertion-mode-appropriately */
  void resetInsertionModeAppropriately();

  /** https://html.spec.whatwg.org/multipage/parsing.html#reconstruct-the-active-formatting-elements */
  void reconstructActiveFormattingElements();

  /** https://html.spec.whatwg.org/multipage/parsing.html#insert-a-character */
  void insertCharacter(std::unique_ptr<CharacterToken> token);

  /** https://html.spec.whatwg.org/multipage/parsing.html#insert-a-comment */
  void insertComment(std::unique_ptr<Token> token,
                     std::shared_ptr<LibDOM::Node> position);

  /** https://dom.spec.whatwg.org/#concept-create-element */
  std::shared_ptr<LibDOM::HTMLElement> createElement(std::wstring localName,
                                                     std::wstring ns,
                                                     std::wstring prefix = L"");

  /** https://html.spec.whatwg.org/multipage/parsing.html#create-an-element-for-the-token */
  std::shared_ptr<LibDOM::Element>
  createElementForToken(TagToken *token, std::wstring ns,
                        std::shared_ptr<LibDOM::Node> intendedParent);

  /** https://html.spec.whatwg.org/multipage/parsing.html#insert-a-foreign-element */
  std::shared_ptr<LibDOM::Element>
  insertForeignElement(TagToken *token, std::wstring ns,
                       bool onlyAddToElementStack);

  /** https://html.spec.whatwg.org/multipage/parsing.html#generic-raw-text-element-parsing-algorithm */
  void genericRawTextParse(std::unique_ptr<TagToken> token);

  /** https://html.spec.whatwg.org/multipage/parsing.html#generic-rcdata-element-parsing-algorithm */
  void genericRcdataParse(std::unique_ptr<TagToken> token);

  /** https://html.spec.whatwg.org/multipage/parsing.html#stop-parsing */
  void stopParsing();

  /** https://html.spec.whatwg.org/multipage/parsing.html#generate-implied-end-tags */
  void generateImpliedEndTags();
  void generateImpliedEndTagsExceptFor(std::wstring tagName);

  /** https://html.spec.whatwg.org/multipage/parsing.html#close-a-p-element */
  void closePElem();

  /** https://html.spec.whatwg.org/multipage/parsing.html#push-onto-the-list-of-active-formatting-elements */
  void pushOntoActiveFormattingElems(std::shared_ptr<LibDOM::Element> node);

  /** https://html.spec.whatwg.org/multipage/parsing.html#has-an-element-in-scope */
  bool isInScope(std::shared_ptr<LibDOM::Element> targetNode);
  bool isInButtonScope(std::shared_ptr<LibDOM::Element> targetNode);
  bool stackHasInScope(std::wstring tagName);
  bool stackHasInButtonScope(std::wstring tagName);

  /** https://html.spec.whatwg.org/multipage/parsing.html#adoption-agency-algorithm

    why is it named the "adoption agency" algo???
  */
  void adoptionAgency(TagToken *token);

  void popStackUntil(std::wstring tagName);

  Tokenizer m_tokenizer;
  ParserMode m_insertionMode = INITIAL;
  ParserMode m_originalInsertionMode = UNDEFINED_MODE;
  /** Stack of open elements */
  std::vector<std::shared_ptr<LibDOM::Element>> m_nodeStack;
  std::vector<std::shared_ptr<LibDOM::Element>> m_activeFormattingElems;

  std::shared_ptr<LibDOM::Element> m_headElementPointer = nullptr;
  std::shared_ptr<LibDOM::Element> m_formElementPointer = nullptr;

  bool m_scriptingFlag = false;
  bool m_framesetOk = true;
  bool m_isParsing = true;
};

} // namespace LibHTML

#endif
