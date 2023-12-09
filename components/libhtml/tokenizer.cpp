#include "libhtml.h"
#include <cctype>
#include <cstdio>
#include <ios>
#include <iostream>
#include <string.h>
#include <strings.h>

namespace LibHTML {

void Tokenizer::process(const char *input, size_t size) {
  // Walk the input using the state machine described by the HTML spec
  // https://html.spec.whatwg.org/#tokenization
  this->input = input;
  input_size = size;
  input_ptr = 0;

  while (input_ptr < input_size) {
    std::cout << "[LibHTML] state=" << current_state << " char=";
    std::cout << std::hex << (int)current_char << std::dec;
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
      emit(START_TAG, "");
      input_ptr--; // reconsume
      current_state = TAG_NAME;
      return;
    }
    if (current_char == '?') {
      // "This is an unexpected-question-mark-instead-of-tag-name parse error.
      // Create a comment token whose data is the empty string. Reconsume in the
      // bogus comment state."
      emit(COMMENT, "");
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

  case MARKUP_DECLARATION: {
    if (strncmp(&input[input_ptr], "--", 2) == 0) {
      consume(2);
      emit(COMMENT, "");
      current_state = COMMENT_START;
      return;
    }
    if (strncasecmp(&input[input_ptr], "DOCTYPE", 7) == 0) {
      consume(7);
      current_state = DOCTYPE;
      return;
    }
    if (strncmp(&input[input_ptr], "[CDATA[", 7) == 0) {
      consume(7);
      // FIXME: handle CDATA properly
      // "this is a cdata-in-html-content parse error. Create a comment token
      // whose data is the "[CDATA[" string. Switch to the bogus comment state."
      emit(COMMENT, "[CDATA[");
      current_state = BOGUS_COMMENT;
      return;
    }
    // "This is an incorrectly-opened-comment parse error. Create a comment
    // token whose data is the empty string. Switch to the bogus comment state
    // (don't consume anything in the current state)."
    emit(COMMENT, "");
    current_state = BOGUS_COMMENT;
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
void Tokenizer::consume() { current_char = input[input_ptr++]; }
void Tokenizer::consume(size_t howMany) {
  input_ptr += howMany;
  current_char = input[input_ptr];
}

} // namespace LibHTML
