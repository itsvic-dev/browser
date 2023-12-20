#include "libhtml/tokenizer.h"
#include "libhtml.h"
#include "libhtml/tokens.h"
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cwchar>
#include <ios>
#include <iostream>
#include <memory>
#include <string.h>
#include <utility>

// helper macros
#define IF_IS(x) if (m_currentChar == (x))
#define RECONSUME m_inputPtr--
#define CONVERT_TO(type, ptr)                                                  \
  std::unique_ptr<type>(reinterpret_cast<type *>(ptr.release()))
#define MOVE_TOKEN(ptr) m_currentToken = std::move(ptr)

namespace LibHTML {

void Tokenizer::process(const wchar_t *input, size_t size,
                        OnEmitFunction onEmit) {
  // Walk the m_input using the state machine described by the HTML spec
  // https://html.spec.whatwg.org/#tokenization
  m_input = input;
  m_inputSize = size;
  m_inputPtr = 0;
  m_onEmit = onEmit;

  while (m_inputPtr < m_inputSize) {
    stateTick();
  }
}

void Tokenizer::stateTick() {
  switch (m_currentState) {
  // https://html.spec.whatwg.org/multipage/parsing.html#data-state
  case DATA: {
    consume();
    switch (m_currentChar) {
    case '&':
      m_returnState = DATA;
      m_currentState = CHARACTER_REFERENCE;
      break;
    case '<':
      m_currentState = TAG_OPEN;
      break;
    case 0:
      // "This is an unexpected-null-character parse error. Emit the current
      // m_input character as a character token."
      emit(std::make_unique<CharacterToken>(m_currentChar));
      break;
    case EOF:
      emit(std::make_unique<EOFToken>());
      break;
    default:
      emit(std::make_unique<CharacterToken>(m_currentChar));
    }
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#tag-open-state
  case TAG_OPEN: {
    consume();
    if (m_currentChar == '!') {
      m_currentState = MARKUP_DECLARATION;
      return;
    }
    if (m_currentChar == '/') {
      m_currentState = END_TAG_OPEN;
      return;
    }
    if (isalpha(m_currentChar)) {
      create(std::make_unique<TagToken>(START_TAG));
      RECONSUME;
      m_currentState = TAG_NAME;
      return;
    }
    if (m_currentChar == '?') {
      // "This is an unexpected-question-mark-instead-of-tag-name parse error.
      // Create a comment token whose data is the empty string. Reconsume in the
      // bogus comment state."
      create(std::make_unique<CommentToken>());
      RECONSUME;
      m_currentState = BOGUS_COMMENT;
      return;
    }
    if (m_currentChar == EOF) {
      // "This is an eof-before-tag-name parse error. Emit a U+003C LESS-THAN
      // SIGN character token and an end-of-file token."
      emit(std::make_unique<CharacterToken>('>'));
      emit(std::make_unique<EOFToken>());
      return;
    }
    // "This is an invalid-first-character-of-tag-name parse error. Emit a
    // U+003C LESS-THAN SIGN character token. Reconsume in the data state."
    emit(std::make_unique<CharacterToken>('>'));
    RECONSUME;
    m_currentState = DATA;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#tag-name-state
  case TAG_NAME: {
    consume();
    if (isspace(m_currentChar)) {
      m_currentState = BEFORE_ATTRIBUTE_NAME;
      return;
    }
    if (m_currentChar == '/') {
      m_currentState = SELF_CLOSING_START_TAG;
      return;
    }
    if (m_currentChar == '>') {
      m_currentState = DATA;
      emitCurrent();
      return;
    }
    if (isupper(m_currentChar)) {
      assert(m_currentToken->type() == START_TAG ||
             m_currentToken->type() == END_TAG);
      auto token = CONVERT_TO(TagToken, m_currentToken);
      token->appendName(m_currentChar + 0x20);
      MOVE_TOKEN(token);
      return;
    }
    if (m_currentChar == 0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current tag token's tag name."
      // FIXME: proper unicode
      assert(m_currentToken->type() == START_TAG ||
             m_currentToken->type() == END_TAG);
      auto token = CONVERT_TO(TagToken, m_currentToken);
      token->appendName(L"\ufffd");
      MOVE_TOKEN(token);
      return;
    }
    if (m_currentChar == EOF) {
      // "This is an eof-in-tag parse error. Emit an end-of-file token."
      emit(std::make_unique<EOFToken>());
      return;
    }
    assert(m_currentToken->type() == START_TAG ||
           m_currentToken->type() == END_TAG);
    auto token = CONVERT_TO(TagToken, m_currentToken);
    token->appendName(m_currentChar);
    MOVE_TOKEN(token);
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#end-tag-open-state
  case END_TAG_OPEN: {
    consume();
    if (isalpha(m_currentChar)) {
      create(std::make_unique<TagToken>(END_TAG));
      RECONSUME;
      m_currentState = TAG_NAME;
      return;
    }
    IF_IS('>') {
      // "This is a missing-end-tag-name parse error. Switch to the data state."
      m_currentState = DATA;
      return;
    }
    IF_IS(EOF) {
      // "This is an eof-before-tag-name parse error. Emit a U+003C LESS-THAN
      // SIGN character token, a U+002F SOLIDUS character token and an
      // end-of-file token."
      emit(std::make_unique<CharacterToken>('<'));
      emit(std::make_unique<CharacterToken>('/'));
      emit(std::make_unique<EOFToken>());
      return;
    }
    // "This is an invalid-first-character-of-tag-name parse error. Create a
    // comment token whose data is the empty string. Reconsume in the bogus
    // comment state."
    create(std::make_unique<CommentToken>());
    m_currentState = BOGUS_COMMENT;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#before-attribute-name-state
  case BEFORE_ATTRIBUTE_NAME: {
    consume();
    if (isspace(m_currentChar)) {
      return; // ignore
    }
    if (m_currentChar == '/' || m_currentChar == '>' || m_currentChar == EOF) {
      RECONSUME;
      m_currentState = AFTER_ATTRIBUTE_NAME;
      return;
    }
    IF_IS('=') {
      assert(m_currentToken->type() == START_TAG);
      auto token = CONVERT_TO(TagToken, m_currentToken);
      token->attributes.push_back({std::wstring(1, m_currentChar), L""});
      m_currentState = ATTRIBUTE_NAME;
      MOVE_TOKEN(token);
      return;
    }
    assert(m_currentToken->type() == START_TAG);
    auto token = CONVERT_TO(TagToken, m_currentToken);
    token->attributes.push_back({L"", L""});
    RECONSUME;
    m_currentState = ATTRIBUTE_NAME;
    MOVE_TOKEN(token);
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#attribute-name-state
  case ATTRIBUTE_NAME: {
    consume();
    if (isspace(m_currentChar) || m_currentChar == '/' ||
        m_currentChar == '>' || m_currentChar == EOF) {
      RECONSUME;
      m_currentState = AFTER_ATTRIBUTE_NAME;
      return;
    }
    IF_IS('=') {
      m_currentState = BEFORE_ATTRIBUTE_VALUE;
      return;
    }
    if (isupper(m_currentChar)) {
      assert(m_currentToken->type() == START_TAG);
      auto token = CONVERT_TO(TagToken, m_currentToken);
      token->attributes.back().name += m_currentChar;
      MOVE_TOKEN(token);
      return;
    }
    IF_IS(0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current attribute's name."
      assert(m_currentToken->type() == START_TAG);
      auto token = CONVERT_TO(TagToken, m_currentToken);
      token->attributes.back().name += L"\ufffd";
      MOVE_TOKEN(token);
      return;
    }
    assert(m_currentToken->type() == START_TAG);
    auto token = CONVERT_TO(TagToken, m_currentToken);
    token->attributes.back().name += m_currentChar;
    MOVE_TOKEN(token);
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#after-attribute-name-state
  case AFTER_ATTRIBUTE_NAME: {
    consume();
    if (isspace(m_currentChar)) {
      return; // ignore
    }
    IF_IS('/') {
      m_currentState = SELF_CLOSING_START_TAG;
      return;
    }
    IF_IS('=') {
      m_currentState = BEFORE_ATTRIBUTE_VALUE;
      return;
    }
    IF_IS(EOF) {
      // This is an eof-in-tag parse error. Emit an end-of-file token.
      emit(std::make_unique<EOFToken>());
      return;
    }
    assert(m_currentToken->type() == START_TAG);
    auto token = CONVERT_TO(TagToken, m_currentToken);
    token->attributes.push_back({L"", L""});
    RECONSUME;
    MOVE_TOKEN(token);
    m_currentState = ATTRIBUTE_NAME;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#before-attribute-value-state
  case BEFORE_ATTRIBUTE_VALUE: {
    consume();
    if (isspace(m_currentChar)) {
      return; // ignore
    }
    IF_IS('"') {
      m_currentState = ATTRIBUTE_VALUE_DOUBLE_QUOTE;
      return;
    }
    IF_IS('\'') {
      m_currentState = ATTRIBUTE_VALUE_SINGLE_QUOTE;
      return;
    }
    IF_IS('>') {
      // "This is a missing-attribute-value parse error. Switch to the data
      // state. Emit the current tag token."
      m_currentState = DATA;
      emitCurrent();
      return;
    }
    RECONSUME;
    m_currentState = ATTRIBUTE_VALUE_UNQUOTED;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(double-quoted)-state
  case ATTRIBUTE_VALUE_DOUBLE_QUOTE: {
    consume();
    IF_IS('"') {
      m_currentState = AFTER_ATTRIBUTE_VALUE_QUOTED;
      return;
    }
    IF_IS('&') {
      m_returnState = ATTRIBUTE_VALUE_DOUBLE_QUOTE;
      m_currentState = CHARACTER_REFERENCE;
      return;
    }
    IF_IS(0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current attribute's value."
      assert(m_currentToken->type() == START_TAG);
      auto token = CONVERT_TO(TagToken, m_currentToken);
      token->attributes.back().value += L"\ufffd";
      MOVE_TOKEN(token);
      return;
    }
    IF_IS(EOF) {
      emit(std::make_unique<EOFToken>());
      return;
    }
    assert(m_currentToken->type() == START_TAG);
    auto token = CONVERT_TO(TagToken, m_currentToken);
    token->attributes.back().value += m_currentChar;
    MOVE_TOKEN(token);
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(single-quoted)-state
  case ATTRIBUTE_VALUE_SINGLE_QUOTE: {
    consume();
    IF_IS('\'') {
      m_currentState = AFTER_ATTRIBUTE_VALUE_QUOTED;
      return;
    }
    IF_IS('&') {
      m_returnState = ATTRIBUTE_VALUE_SINGLE_QUOTE;
      m_currentState = CHARACTER_REFERENCE;
      return;
    }
    IF_IS(0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current attribute's value."
      assert(m_currentToken->type() == START_TAG);
      auto token = CONVERT_TO(TagToken, m_currentToken);
      token->attributes.back().value += L"\ufffd";
      MOVE_TOKEN(token);
      return;
    }
    IF_IS(EOF) {
      emit(std::make_unique<EOFToken>());
      return;
    }
    assert(m_currentToken->type() == START_TAG);
    auto token = CONVERT_TO(TagToken, m_currentToken);
    token->attributes.back().value += m_currentChar;
    MOVE_TOKEN(token);
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(unquoted)-state
  case ATTRIBUTE_VALUE_UNQUOTED: {
    consume();
    if (isspace(m_currentChar)) {
      m_currentState = BEFORE_ATTRIBUTE_NAME;
      return;
    }
    IF_IS('&') {
      m_returnState = ATTRIBUTE_VALUE_UNQUOTED;
      m_currentState = CHARACTER_REFERENCE;
      return;
    }
    IF_IS('>') {
      m_currentState = DATA;
      emitCurrent();
      return;
    }
    IF_IS(0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current attribute's value."
      assert(m_currentToken->type() == START_TAG);
      auto token = CONVERT_TO(TagToken, m_currentToken);
      token->attributes.back().value += L"\ufffd";
      MOVE_TOKEN(token);
      return;
    }
    IF_IS(EOF) {
      emit(std::make_unique<EOFToken>());
      return;
    }
    assert(m_currentToken->type() == START_TAG);
    auto token = CONVERT_TO(TagToken, m_currentToken);
    token->attributes.back().value += m_currentChar;
    MOVE_TOKEN(token);
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#after-attribute-value-(quoted)-state
  case AFTER_ATTRIBUTE_VALUE_QUOTED: {
    consume();
    if (isspace(m_currentChar)) {
      m_currentState = BEFORE_ATTRIBUTE_NAME;
      return;
    }
    IF_IS('/') {
      m_currentState = SELF_CLOSING_START_TAG;
      return;
    }
    IF_IS('>') {
      m_currentState = DATA;
      emitCurrent();
      return;
    }
    IF_IS(EOF) {
      // "This is an eof-in-tag parse error. Emit an end-of-file token."
      emit(std::make_unique<EOFToken>());
      return;
    }
    // "This is a missing-whitespace-between-attributes parse error. Reconsume
    // in the before attribute name state."
    RECONSUME;
    m_currentState = BEFORE_ATTRIBUTE_NAME;
    break;
  }

  case SELF_CLOSING_START_TAG: {
    consume();
    IF_IS('>') {
      assert(m_currentToken->type() == START_TAG);
      auto token = CONVERT_TO(TagToken, m_currentToken);
      token->selfClosing = true;
      MOVE_TOKEN(token);
      emitCurrent();
      return;
    }
    IF_IS(EOF) {
      emit(std::make_unique<EOFToken>());
      return;
    }
    // "This is an unexpected-solidus-in-tag parse error. Reconsume in the
    // before attribute name state."
    RECONSUME;
    m_currentState = BEFORE_ATTRIBUTE_NAME;
    break;
  }

  case MARKUP_DECLARATION: {
    if (wcsncmp(&m_input[m_inputPtr], L"--", 2) == 0) {
      consume(2);
      emit(std::make_unique<CommentToken>());
      m_currentState = COMMENT_START;
      return;
    }
    if (wcsncasecmp(&m_input[m_inputPtr], L"DOCTYPE", 7) == 0) {
      consume(7);
      m_currentState = DOCTYPE_STATE;
      return;
    }
    if (wcsncmp(&m_input[m_inputPtr], L"[CDATA[", 7) == 0) {
      consume(7);
      // FIXME: handle CDATA properly
      // "this is a cdata-in-html-content parse error. Create a comment token
      // whose data is the "[CDATA[" string. Switch to the bogus comment state."
      create(std::make_unique<CommentToken>(L"[CDATA["));
      m_currentState = BOGUS_COMMENT;
      return;
    }
    // "This is an incorrectly-opened-comment parse error. Create a comment
    // token whose data is the empty string. Switch to the bogus comment state
    // (don't consume anything in the current state)."
    create(std::make_unique<CommentToken>());
    m_currentState = BOGUS_COMMENT;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#doctype-state
  case DOCTYPE_STATE: {
    consume();
    if (isspace(m_currentChar)) {
      m_currentState = BEFORE_DOCTYPE_NAME;
      return;
    }
    if (m_currentChar == '>') {
      RECONSUME;
      m_currentState = BEFORE_DOCTYPE_NAME;
      return;
    }
    if (m_currentChar == EOF) {
      // "This is an eof-in-doctype parse error. Create a new DOCTYPE token. Set
      // its force-quirks flag to on. Emit the current token. Emit an
      // end-of-file token."
      auto token = std::make_unique<DoctypeToken>();
      token->setForceQuirks(true);
      emit(std::move(token));
      emit(std::make_unique<EOFToken>());
      return;
    }
    // "This is a missing-whitespace-before-doctype-name parse error. Reconsume
    // in the before DOCTYPE name state."
    RECONSUME;
    m_currentState = BEFORE_DOCTYPE_NAME;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#before-doctype-name-state
  case BEFORE_DOCTYPE_NAME: {
    consume();
    if (isspace(m_currentChar)) {
      return; // ignore
    }
    if (isupper(m_currentChar)) {
      create(std::make_unique<DoctypeToken>(m_currentChar + 0x20));
      m_currentState = DOCTYPE_NAME;
      return;
    }
    if (m_currentChar == 0) {
      // "This is an unexpected-null-character parse error. Create a new DOCTYPE
      // token. Set the token's name to a U+FFFD REPLACEMENT CHARACTER
      // character. Switch to the DOCTYPE name state."
      create(std::make_unique<DoctypeToken>(L"\ufffd"));
      m_currentState = DOCTYPE_NAME;
      return;
    }
    if (m_currentChar == '>') {
      // "This is a missing-doctype-name parse error. Create a new DOCTYPE
      // token. Set its force-quirks flag to on. Switch to the data state. Emit
      // the current token."
      // emit(DOCTYPE_TOKEN, "");
      auto token = std::make_unique<DoctypeToken>();
      token->setForceQuirks(true);
      m_currentState = DATA;
      emit(std::move(token));
      return;
    }
    if (m_currentChar == EOF) {
      // "This is an eof-in-doctype parse error. Create a new DOCTYPE token. Set
      // its force-quirks flag to on. Emit the current token. Emit an
      // end-of-file token."
      auto token = std::make_unique<DoctypeToken>();
      token->setForceQuirks(true);
      emit(std::move(token));
      emit(std::make_unique<EOFToken>());
      return;
    }

    create(std::make_unique<DoctypeToken>(m_currentChar));
    m_currentState = DOCTYPE_NAME;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#doctype-name-state
  case DOCTYPE_NAME: {
    consume();
    if (isspace(m_currentChar)) {
      m_currentState = AFTER_DOCTYPE_NAME;
      return;
    }
    if (m_currentChar == '>') {
      m_currentState = DATA;
      emitCurrent();
    }
    if (isupper(m_currentChar)) {
      assert(m_currentToken->type() == DOCTYPE_TOKEN);
      auto token = CONVERT_TO(DoctypeToken, m_currentToken);
      token->appendName(m_currentChar + 0x20);
      MOVE_TOKEN(token);
    }
    if (m_currentChar == 0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current DOCTYPE token's name.
      assert(m_currentToken->type() == DOCTYPE_TOKEN);
      auto token = CONVERT_TO(DoctypeToken, m_currentToken);
      token->appendName(L"\ufffd");
      MOVE_TOKEN(token);
    }
    if (m_currentChar == EOF) {
      // "This is an eof-in-doctype parse error. Set the current DOCTYPE token's
      // force-quirks flag to on. Emit the current DOCTYPE token. Emit an
      // end-of-file token."
      assert(m_currentToken->type() == DOCTYPE_TOKEN);
      auto token = CONVERT_TO(DoctypeToken, m_currentToken);
      token->setForceQuirks(true);
      MOVE_TOKEN(token);
      emitCurrent();
      emit(std::make_unique<EOFToken>());
    }
    assert(m_currentToken->type() == DOCTYPE_TOKEN);
    auto token = CONVERT_TO(DoctypeToken, m_currentToken);
    token->appendName(m_currentChar);
    MOVE_TOKEN(token);
    break;
  }

  // Unhandled state - missing implementation
  default: {
    std::cerr << "[LibHTML] Unhandled state " << m_currentState << "\n";
    throw 0;
  }
  }
}

void Tokenizer::emit(std::unique_ptr<Token> token) {
  m_onEmit(std::move(token));
}
void Tokenizer::create(std::unique_ptr<Token> token) {
  m_currentToken = std::move(token);
}
void Tokenizer::emitCurrent() { emit(std::move(m_currentToken)); }

void Tokenizer::consume() { m_currentChar = m_input[m_inputPtr++]; }
void Tokenizer::consume(size_t howMany) {
  m_inputPtr += howMany;
  m_currentChar = m_input[m_inputPtr];
}

} // namespace LibHTML
