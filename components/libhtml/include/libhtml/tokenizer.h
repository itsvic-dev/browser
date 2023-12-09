#pragma once
#include "libhtml/tokens.h"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace LibHTML {

enum TokenizerState : int {
  UNDEFINED_STATE,
  DATA,
  TAG_OPEN,
  TAG_NAME,
  END_TAG_OPEN,
  BEFORE_ATTRIBUTE_NAME,
  ATTRIBUTE_NAME,
  AFTER_ATTRIBUTE_NAME,
  BEFORE_ATTRIBUTE_VALUE,
  ATTRIBUTE_VALUE_DOUBLE_QUOTE,
  ATTRIBUTE_VALUE_SINGLE_QUOTE,
  ATTRIBUTE_VALUE_UNQUOTED,
  AFTER_ATTRIBUTE_VALUE_QUOTED,
  AFTER_ATTRIBUTE_VALUE_UNQUOTED,
  SELF_CLOSING_START_TAG,
  CHARACTER_REFERENCE,
  MARKUP_DECLARATION,
  BOGUS_COMMENT,
  COMMENT_START,
  DOCTYPE_STATE,
  BEFORE_DOCTYPE_NAME,
  DOCTYPE_NAME,
  AFTER_DOCTYPE_NAME,
};

class Tokenizer {
public:
  Tokenizer() = default;
  ~Tokenizer() = default;

  void process(const char *input, size_t size);

  /**
    Get the amount of processed characters.
  */
  size_t processed();

private:
  void stateTick();
  void consume();
  void consume(size_t howMany);
  void emit(std::shared_ptr<Token> token);
  void create(std::shared_ptr<Token> token);
  void emitCurrent();

  TokenizerState current_state = DATA;
  TokenizerState return_state = UNDEFINED_STATE;

  const char *input = nullptr;
  size_t input_size = 0;
  size_t input_ptr = 0;
  char current_char = 0;

  std::shared_ptr<Token> current_token;
  std::vector<std::shared_ptr<Token>> tokens = {};
};

} // namespace LibHTML
