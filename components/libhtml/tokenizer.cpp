#include "libhtml/tokenizer.h"
#include "libhtml.h"
#include <cctype>
#include <cstdio>
#include <ios>
#include <iostream>
#include <string.h>
#include <strings.h>

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
    std::cout << std::hex << (int)current_char << std::dec << "/"
              << current_char;
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
      emit(CHARACTER, current_char);
      break;
    case EOF:
      emit(END_OF_FILE, 0);
      break;
    default:
      emit(CHARACTER, current_char);
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
      create(START_TAG, "");
      input_ptr--; // reconsume
      current_state = TAG_NAME;
      return;
    }
    if (current_char == '?') {
      // "This is an unexpected-question-mark-instead-of-tag-name parse error.
      // Create a comment token whose data is the empty string. Reconsume in the
      // bogus comment state."
      create(COMMENT, "");
      input_ptr--; // reconsume
      current_state = BOGUS_COMMENT;
      return;
    }
    if (current_char == EOF) {
      // "This is an eof-before-tag-name parse error. Emit a U+003C LESS-THAN
      // SIGN character token and an end-of-file token."
      emit(CHARACTER, '>');
      emit(END_OF_FILE, "");
      return;
    }
    // "This is an invalid-first-character-of-tag-name parse error. Emit a
    // U+003C LESS-THAN SIGN character token. Reconsume in the data state."
    emit(CHARACTER, '>');
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
      current_token.data += current_char + 0x20;
      return;
    }
    if (current_char == 0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current tag token's tag name."
      // FIXME: proper unicode
      current_token.data += "\xff\xfd";
      return;
    }
    if (current_char == EOF) {
      // "This is an eof-in-tag parse error. Emit an end-of-file token."
      emit(END_OF_FILE, "");
      return;
    }
    current_token.data += current_char;
    break;
  }

  // https://html.spec.whatwg.org/multipage/parsing.html#end-tag-open-state
  case END_TAG_OPEN: {
    consume();
    if (isalpha(current_char)) {
      create(END_TAG, "");
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
      emit(CHARACTER, '<');
      emit(CHARACTER, '/');
      emit(END_OF_FILE, "");
      return;
    }
    // "This is an invalid-first-character-of-tag-name parse error. Create a
    // comment token whose data is the empty string. Reconsume in the bogus
    // comment state."
    create(COMMENT, "");
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
      // TODO: start a new attribute in the tag token
      // i should make a Tag class with derivatives (such as TagToken or
      // DoctypeToken)
      current_state = ATTRIBUTE_NAME;
      return;
    }
    // TODO: start a new attribute in the tag token
    input_ptr--;
    current_state = ATTRIBUTE_NAME;
    break;
  }

  case MARKUP_DECLARATION: {
    if (strncmp(&input[input_ptr], "--", 2) == 0) {
      consume(2);
      emit(COMMENT, "");
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
      create(COMMENT, "[CDATA[");
      current_state = BOGUS_COMMENT;
      return;
    }
    // "This is an incorrectly-opened-comment parse error. Create a comment
    // token whose data is the empty string. Switch to the bogus comment state
    // (don't consume anything in the current state)."
    create(COMMENT, "");
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
      emit(DOCTYPE_TOKEN, ""); // FIXME: force-quirks flag
      emit(END_OF_FILE, "");
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
      create(DOCTYPE_TOKEN, current_char + 0x20);
      current_state = DOCTYPE_NAME;
      return;
    }
    if (current_char == 0) {
      // "This is an unexpected-null-character parse error. Create a new DOCTYPE
      // token. Set the token's name to a U+FFFD REPLACEMENT CHARACTER
      // character. Switch to the DOCTYPE name state."
      // FIXME: proper unicode support
      create(DOCTYPE_TOKEN, "\xff\xfd");
      current_state = DOCTYPE_NAME;
      return;
    }
    if (current_char == '>') {
      // "This is a missing-doctype-name parse error. Create a new DOCTYPE
      // token. Set its force-quirks flag to on. Switch to the data state. Emit
      // the current token."
      // FIXME: force-quirks flag
      emit(DOCTYPE_TOKEN, "");
      current_state = DATA;
      return;
    }
    if (current_char == EOF) {
      // "This is an eof-in-doctype parse error. Create a new DOCTYPE token. Set
      // its force-quirks flag to on. Emit the current token. Emit an
      // end-of-file token."
      // FIXME: force-quirks flag
      emit(DOCTYPE_TOKEN, "");
      emit(END_OF_FILE, "");
      return;
    }

    create(DOCTYPE_TOKEN, current_char);
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
      current_token.data += current_char + 0x20;
    }
    if (current_char == 0) {
      // "This is an unexpected-null-character parse error. Append a U+FFFD
      // REPLACEMENT CHARACTER character to the current DOCTYPE token's name."
      // FIXME: proper unicode
      current_token.data += "\xff\xfd";
    }
    if (current_char == EOF) {
      // "This is an eof-in-doctype parse error. Set the current DOCTYPE token's
      // force-quirks flag to on. Emit the current DOCTYPE token. Emit an
      // end-of-file token."
      // FIXME: force-quirks flag
      emitCurrent();
      emit(END_OF_FILE, "");
    }
    current_token.data += current_char;
    break;
  }

  // Unhandled state - missing implementation
  default: {
    std::cerr << "[LibHTML] Unhandled state " << current_state << "\n";
    throw 0;
  }
  }
}

void Tokenizer::emit(TokenType type, const char data) {
  emit(type, std::string(1, data));
}
void Tokenizer::emit(TokenType type, const std::string data) {
  tokens.push_back({type, data});
  std::cout << "[LibHTML] emitting token " << type << " (data " << data
            << ")\n";
}
void Tokenizer::create(TokenType type, const char data) {
  create(type, std::string(1, data));
}
void Tokenizer::create(TokenType type, const std::string data) {
  current_token = Token{type, data};
}
void Tokenizer::emitCurrent() { emit(current_token.type, current_token.data); }

void Tokenizer::consume() { current_char = input[input_ptr++]; }
void Tokenizer::consume(size_t howMany) {
  input_ptr += howMany;
  current_char = input[input_ptr];
}

} // namespace LibHTML
