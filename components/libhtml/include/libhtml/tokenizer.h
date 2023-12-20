#ifndef LIBHTML_TOKENIZER_H
#define LIBHTML_TOKENIZER_H

#include "libhtml/tokens.h"
#include <cstddef>
#include <functional>
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

typedef std::function<void(std::unique_ptr<Token>)> OnEmitFunction;

class Tokenizer {
public:
  Tokenizer() = default;
  ~Tokenizer() = default;

  void process(const wchar_t *input, size_t size, OnEmitFunction onEmit);

  TokenizerState currentState = DATA;
  TokenizerState returnState = UNDEFINED_STATE;

private:
  void stateTick();
  void consume();
  void consume(size_t howMany);
  void emit(std::unique_ptr<Token> token);
  void create(std::unique_ptr<Token> token);
  void emitCurrent();
  OnEmitFunction m_onEmit;

  const wchar_t *m_input = nullptr;
  size_t m_inputSize = 0;
  size_t m_inputPtr = 0;
  wchar_t m_currentChar = 0;

  std::unique_ptr<Token> m_currentToken;
};

} // namespace LibHTML

#endif
