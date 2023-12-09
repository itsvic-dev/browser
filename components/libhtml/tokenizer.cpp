#include "libhtml/tokenizer.h"
#include "libhtml.h"
#include "libhtml/tokens.h"
#include <cassert>
#include <cctype>
#include <cstdio>
#include <ios>
#include <iostream>
#include <memory>
#include <string.h>

// helper macros
#define IF_IS(x) if (current_char == (x))

namespace LibHTML {

void Tokenizer::process(const char *input, size_t size) {
  // Walk the input using the state machine described by the HTML spec
  // https://html.spec.whatwg.org/#tokenization
  this->input = input;
  input_size = size;
  input_ptr = 0;

  while (input_ptr < input_size) {
    std::cout << "[LibHTML] state=" << current_state << " char=";
    std::cout << std::hex << (int)input[input_ptr] << std::dec << "/"
              << input[input_ptr];
    std::cout << " ptr=" << input_ptr << "\n";
    stateTick();
  }
}

size_t Tokenizer::processed() { return input_ptr; }

void Tokenizer::stateTick() {
  switch (current_state) {
  // https://html.spec.whatwg.org/multipage/parsing.html#data-state
  case DATA: {
    consume();
    switch (current_char) {
    case '&':
      return_state = DATA;
      current_state = CHARACTER_REFERENCE;
      break;
    case '<':
      current_state = TAG_OPEN;
      break;
    case 0:
      // "This is an unexpected-null-character parse error. Emit the current
      // input character as a character token."
      emit(std::make_shared<CharacterToken>(current_char));
      break;
    case EOF:
      emit(std::make_shared<EOFToken>());
      break;
    default:
      emit(std::make_shared<CharacterToken>(current_char));
    }
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#tag-open-state
  case TAG_OPEN: {
    consume();
    if (current_char == '!') {
      current_state = MARKUP_DECLARATION;
      return;
    }
    if (current_char == '/') {
      current_state = END_TAG_OPEN;
      return;
    }
    if (isalpha(current_char)) {
      create(std::make_shared<TagToken>(START_TAG));
      input_ptr--; // reconsume
      current_state = TAG_NAME;
      return;
    }
    if (current_char == '?') {
      // "This is an unexpected-question-mark-instead-of-tag-name parse error.
      // Create a comment token whose data is the empty string. Reconsume in the
      // bogus comment state."
      create(std::make_shared<CommentToken>());
      input_ptr--; // reconsume
      current_state = BOGUS_COMMENT;
      return;
    }
    if (current_char == EOF) {
      // "This is an eof-before-tag-name parse error. Emit a U+003C LESS-THAN
      // SIGN character token and an end-of-file token."
      emit(std::make_shared<CharacterToken>('>'));
      emit(std::make_shared<EOFToken>());
      return;
    }
    // "This is an invalid-first-character-of-tag-name parse error. Emit a
    // U+003C LESS-THAN SIGN character token. Reconsume in the data state."
    emit(std::make_shared<CharacterToken>('>'));
    input_ptr--; // reconsume
    current_state = DATA;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#tag-name-state
  case TAG_NAME: {
    consume();
    if (isspace(current_char)) {
      current_state = BEFORE_ATTRIBUTE_NAME;
      return;
    }
    if (current_char == '/') {
      current_state = SELF_CLOSING_START_TAG;
      return;
    }
    if (current_char == '>') {
      current_state = DATA;
      emitCurrent();
      return;
    }
    if (isupper(current_char)) {
      assert(current_token->type() == START_TAG ||
             current_token->type() == END_TAG);
      auto token = std::static_pointer_cast<TagToken>(current_token);
      token->appendName(current_char + 0x20);
      return;
    }
    if (current_char == 0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current tag token's tag name."
      // FIXME: proper unicode
      assert(current_token->type() == START_TAG ||
             current_token->type() == END_TAG);
      auto token = std::static_pointer_cast<TagToken>(current_token);
      token->appendName("\xff\xfd");
      return;
    }
    if (current_char == EOF) {
      // "This is an eof-in-tag parse error. Emit an end-of-file token."
      emit(std::make_shared<EOFToken>());
      return;
    }
    assert(current_token->type() == START_TAG ||
           current_token->type() == END_TAG);
    auto token = std::static_pointer_cast<TagToken>(current_token);
    token->appendName(current_char);
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#end-tag-open-state
  case END_TAG_OPEN: {
    consume();
    if (isalpha(current_char)) {
      create(std::make_shared<TagToken>(END_TAG));
      input_ptr--;
      current_state = TAG_NAME;
      return;
    }
    IF_IS('>') {
      // "This is a missing-end-tag-name parse error. Switch to the data state."
      current_state = DATA;
      return;
    }
    IF_IS(EOF) {
      // "This is an eof-before-tag-name parse error. Emit a U+003C LESS-THAN
      // SIGN character token, a U+002F SOLIDUS character token and an
      // end-of-file token."
      emit(std::make_shared<CharacterToken>('<'));
      emit(std::make_shared<CharacterToken>('/'));
      emit(std::make_shared<EOFToken>());
      return;
    }
    // "This is an invalid-first-character-of-tag-name parse error. Create a
    // comment token whose data is the empty string. Reconsume in the bogus
    // comment state."
    create(std::make_shared<CommentToken>());
    current_state = BOGUS_COMMENT;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#before-attribute-name-state
  case BEFORE_ATTRIBUTE_NAME: {
    consume();
    if (isspace(current_char)) {
      return; // ignore
    }
    if (current_char == '/' || current_char == '>' || current_char == EOF) {
      input_ptr--;
      current_state = AFTER_ATTRIBUTE_NAME;
      return;
    }
    IF_IS('=') {
      assert(current_token->type() == START_TAG);
      auto token = std::static_pointer_cast<TagToken>(current_token);
      token->attributes.push_back({std::string(1, current_char), ""});
      current_state = ATTRIBUTE_NAME;
      return;
    }
    assert(current_token->type() == START_TAG);
    auto token = std::static_pointer_cast<TagToken>(current_token);
    token->attributes.push_back({"", ""});
    input_ptr--;
    current_state = ATTRIBUTE_NAME;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#attribute-name-state
  case ATTRIBUTE_NAME: {
    consume();
    if (isspace(current_char) || current_char == '/' || current_char == '>' ||
        current_char == EOF) {
      input_ptr--;
      current_state = AFTER_ATTRIBUTE_NAME;
      return;
    }
    IF_IS('=') {
      current_state = BEFORE_ATTRIBUTE_VALUE;
      return;
    }
    if (isupper(current_char)) {
      assert(current_token->type() == START_TAG);
      auto token = std::static_pointer_cast<TagToken>(current_token);
      token->attributes.back().name += current_char;
      return;
    }
    IF_IS(0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current attribute's name."
      assert(current_token->type() == START_TAG);
      auto token = std::static_pointer_cast<TagToken>(current_token);
      token->attributes.back().name += "\xff\xfd";
      return;
    }
    assert(current_token->type() == START_TAG);
    auto token = std::static_pointer_cast<TagToken>(current_token);
    token->attributes.back().name += current_char;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#after-attribute-name-state
  case AFTER_ATTRIBUTE_NAME: {
    consume();
    if (isspace(current_char)) {
      return; // ignore
    }
    IF_IS('/') {
      current_state = SELF_CLOSING_START_TAG;
      return;
    }
    IF_IS('=') {
      current_state = BEFORE_ATTRIBUTE_VALUE;
      return;
    }
    IF_IS(EOF) {
      // This is an eof-in-tag parse error. Emit an end-of-file token.
      emit(std::make_shared<EOFToken>());
      return;
    }
    assert(current_token->type() == START_TAG);
    auto token = std::static_pointer_cast<TagToken>(current_token);
    token->attributes.push_back({"", ""});
    input_ptr--;
    current_state = ATTRIBUTE_NAME;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#before-attribute-value-state
  case BEFORE_ATTRIBUTE_VALUE: {
    consume();
    if (isspace(current_char)) {
      return; // ignore
    }
    IF_IS('"') {
      current_state = ATTRIBUTE_VALUE_DOUBLE_QUOTE;
      return;
    }
    IF_IS('\'') {
      current_state = ATTRIBUTE_VALUE_SINGLE_QUOTE;
      return;
    }
    IF_IS('>') {
      // "This is a missing-attribute-value parse error. Switch to the data
      // state. Emit the current tag token."
      current_state = DATA;
      emitCurrent();
      return;
    }
    input_ptr--;
    current_state = ATTRIBUTE_VALUE_UNQUOTED;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(double-quoted)-state
  case ATTRIBUTE_VALUE_DOUBLE_QUOTE: {
    consume();
    IF_IS('"') {
      current_state = AFTER_ATTRIBUTE_VALUE_QUOTED;
      return;
    }
    IF_IS('&') {
      return_state = ATTRIBUTE_VALUE_DOUBLE_QUOTE;
      current_state = CHARACTER_REFERENCE;
      return;
    }
    IF_IS(0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current attribute's value."
      assert(current_token->type() == START_TAG);
      auto token = std::static_pointer_cast<TagToken>(current_token);
      token->attributes.back().value += "\xff\xfd";
      return;
    }
    IF_IS(EOF) {
      emit(std::make_shared<EOFToken>());
      return;
    }
    assert(current_token->type() == START_TAG);
    auto token = std::static_pointer_cast<TagToken>(current_token);
    token->attributes.back().value += current_char;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(single-quoted)-state
  case ATTRIBUTE_VALUE_SINGLE_QUOTE: {
    consume();
    IF_IS('\'') {
      current_state = AFTER_ATTRIBUTE_VALUE_QUOTED;
      return;
    }
    IF_IS('&') {
      return_state = ATTRIBUTE_VALUE_SINGLE_QUOTE;
      current_state = CHARACTER_REFERENCE;
      return;
    }
    IF_IS(0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current attribute's value."
      assert(current_token->type() == START_TAG);
      auto token = std::static_pointer_cast<TagToken>(current_token);
      token->attributes.back().value += "\xff\xfd";
      return;
    }
    IF_IS(EOF) {
      emit(std::make_shared<EOFToken>());
      return;
    }
    assert(current_token->type() == START_TAG);
    auto token = std::static_pointer_cast<TagToken>(current_token);
    token->attributes.back().value += current_char;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#attribute-value-(unquoted)-state
  case ATTRIBUTE_VALUE_UNQUOTED: {
    consume();
    if (isspace(current_char)) {
      current_state = BEFORE_ATTRIBUTE_NAME;
      return;
    }
    IF_IS('&') {
      return_state = ATTRIBUTE_VALUE_UNQUOTED;
      current_state = CHARACTER_REFERENCE;
      return;
    }
    IF_IS('>') {
      current_state = DATA;
      emitCurrent();
      return;
    }
    IF_IS(0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current attribute's value."
      assert(current_token->type() == START_TAG);
      auto token = std::static_pointer_cast<TagToken>(current_token);
      token->attributes.back().value += "\xff\xfd";
      return;
    }
    IF_IS(EOF) {
      emit(std::make_shared<EOFToken>());
      return;
    }
    assert(current_token->type() == START_TAG);
    auto token = std::static_pointer_cast<TagToken>(current_token);
    token->attributes.back().value += current_char;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#after-attribute-value-(quoted)-state
  case AFTER_ATTRIBUTE_VALUE_QUOTED: {
    consume();
    if (isspace(current_char)) {
      current_state = BEFORE_ATTRIBUTE_NAME;
      return;
    }
    IF_IS('/') {
      current_state = SELF_CLOSING_START_TAG;
      return;
    }
    IF_IS('>') {
      current_state = DATA;
      emitCurrent();
      return;
    }
    IF_IS(EOF) {
      // "This is an eof-in-tag parse error. Emit an end-of-file token."
      emit(std::make_shared<EOFToken>());
      return;
    }
    // "This is a missing-whitespace-between-attributes parse error. Reconsume
    // in the before attribute name state."
    input_ptr--;
    current_state = BEFORE_ATTRIBUTE_NAME;
    break;
  }

  case SELF_CLOSING_START_TAG: {
    consume();
    IF_IS('>') {
      assert(current_token->type() == START_TAG);
      auto token = std::static_pointer_cast<TagToken>(current_token);
      token->selfClosing = true;
      emitCurrent();
      return;
    }
    IF_IS(EOF) {
      emit(std::make_shared<EOFToken>());
      return;
    }
    // "This is an unexpected-solidus-in-tag parse error. Reconsume in the
    // before attribute name state."
    input_ptr--;
    current_state = BEFORE_ATTRIBUTE_NAME;
    break;
  }

  case MARKUP_DECLARATION: {
    if (strncmp(&input[input_ptr], "--", 2) == 0) {
      consume(2);
      emit(std::make_shared<CommentToken>());
      current_state = COMMENT_START;
      return;
    }
    if (strncasecmp(&input[input_ptr], "DOCTYPE", 7) == 0) {
      consume(7);
      current_state = DOCTYPE_STATE;
      return;
    }
    if (strncmp(&input[input_ptr], "[CDATA[", 7) == 0) {
      consume(7);
      // FIXME: handle CDATA properly
      // "this is a cdata-in-html-content parse error. Create a comment token
      // whose data is the "[CDATA[" string. Switch to the bogus comment state."
      create(std::make_shared<CommentToken>("[CDATA["));
      current_state = BOGUS_COMMENT;
      return;
    }
    // "This is an incorrectly-opened-comment parse error. Create a comment
    // token whose data is the empty string. Switch to the bogus comment state
    // (don't consume anything in the current state)."
    create(std::make_shared<CommentToken>());
    current_state = BOGUS_COMMENT;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#doctype-state
  case DOCTYPE_STATE: {
    consume();
    if (isspace(current_char)) {
      current_state = BEFORE_DOCTYPE_NAME;
      return;
    }
    if (current_char == '>') {
      input_ptr--; // reconsume
      current_state = BEFORE_DOCTYPE_NAME;
      return;
    }
    if (current_char == EOF) {
      // "This is an eof-in-doctype parse error. Create a new DOCTYPE token. Set
      // its force-quirks flag to on. Emit the current token. Emit an
      // end-of-file token."
      auto token = std::make_shared<DoctypeToken>();
      token->setForceQuirks(true);
      emit(token);
      emit(std::make_shared<EOFToken>());
      return;
    }
    // "This is a missing-whitespace-before-doctype-name parse error. Reconsume
    // in the before DOCTYPE name state."
    input_ptr--;
    current_state = BEFORE_DOCTYPE_NAME;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#before-doctype-name-state
  case BEFORE_DOCTYPE_NAME: {
    consume();
    if (isspace(current_char)) {
      return; // ignore
    }
    if (isupper(current_char)) {
      create(std::make_shared<DoctypeToken>(current_char + 0x20));
      current_state = DOCTYPE_NAME;
      return;
    }
    if (current_char == 0) {
      // "This is an unexpected-null-character parse error. Create a new DOCTYPE
      // token. Set the token's name to a U+FFFD REPLACEMENT CHARACTER
      // character. Switch to the DOCTYPE name state."
      // FIXME: proper unicode support
      create(std::make_shared<DoctypeToken>("\xff\xfd"));
      current_state = DOCTYPE_NAME;
      return;
    }
    if (current_char == '>') {
      // "This is a missing-doctype-name parse error. Create a new DOCTYPE
      // token. Set its force-quirks flag to on. Switch to the data state. Emit
      // the current token."
      // emit(DOCTYPE_TOKEN, "");
      auto token = std::make_shared<DoctypeToken>();
      token->setForceQuirks(true);
      current_state = DATA;
      emit(token);
      return;
    }
    if (current_char == EOF) {
      // "This is an eof-in-doctype parse error. Create a new DOCTYPE token. Set
      // its force-quirks flag to on. Emit the current token. Emit an
      // end-of-file token."
      auto token = std::make_shared<DoctypeToken>();
      token->setForceQuirks(true);
      emit(token);
      emit(std::make_shared<EOFToken>());
      return;
    }

    create(std::make_shared<DoctypeToken>(current_char));
    current_state = DOCTYPE_NAME;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#doctype-name-state
  case DOCTYPE_NAME: {
    consume();
    if (isspace(current_char)) {
      current_state = AFTER_DOCTYPE_NAME;
      return;
    }
    if (current_char == '>') {
      current_state = DATA;
      emitCurrent();
    }
    if (isupper(current_char)) {
      assert(current_token->type() == DOCTYPE_TOKEN);
      auto token = std::static_pointer_cast<DoctypeToken>(current_token);
      token->appendName(current_char + 0x20);
    }
    if (current_char == 0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current DOCTYPE token's name."
      // FIXME: proper unicode
      assert(current_token->type() == DOCTYPE_TOKEN);
      auto token = std::static_pointer_cast<DoctypeToken>(current_token);
      token->appendName("\xff\xfd");
    }
    if (current_char == EOF) {
      // "This is an eof-in-doctype parse error. Set the current DOCTYPE token's
      // force-quirks flag to on. Emit the current DOCTYPE token. Emit an
      // end-of-file token."
      assert(current_token->type() == DOCTYPE_TOKEN);
      auto token = std::static_pointer_cast<DoctypeToken>(current_token);
      token->setForceQuirks(true);
      emitCurrent();
      emit(std::make_shared<EOFToken>());
    }
    assert(current_token->type() == DOCTYPE_TOKEN);
    auto token = std::static_pointer_cast<DoctypeToken>(current_token);
    token->appendName(current_char);
    break;
  }

  // Unhandled state - missing implementation
  default: {
    std::cerr << "[LibHTML] Unhandled state " << current_state << "\n";
    throw 0;
  }
  }
}

void Tokenizer::emit(std::shared_ptr<Token> token) {
  tokens.push_back(token);
  std::cout << "[LibHTML] emitting token " << token->type() << "\n";
}
void Tokenizer::create(std::shared_ptr<Token> token) { current_token = token; }
void Tokenizer::emitCurrent() { emit(current_token); }

void Tokenizer::consume() { current_char = input[input_ptr++]; }
void Tokenizer::consume(size_t howMany) {
  input_ptr += howMany;
  current_char = input[input_ptr];
}

} // namespace LibHTML
