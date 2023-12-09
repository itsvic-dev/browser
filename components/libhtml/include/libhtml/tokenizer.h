#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace LibHTML {

typedef enum {
  UNDEFINED,
  DATA,
  TAG_OPEN,
  TAG_NAME,
  END_TAG_OPEN,
  BEFORE_ATTRIBUTE_NAME,
  ATTRIBUTE_NAME,
  AFTER_ATTRIBUTE_NAME,
  SELF_CLOSING_START_TAG,
  CHARACTER_REFERENCE,
  MARKUP_DECLARATION,
  BOGUS_COMMENT,
  COMMENT_START,
  DOCTYPE_STATE,
  BEFORE_DOCTYPE_NAME,
  DOCTYPE_NAME,
  AFTER_DOCTYPE_NAME,
} TokenizerState;

typedef enum {
  CHARACTER,
  END_OF_FILE,
  START_TAG,
  END_TAG,
  COMMENT,
  DOCTYPE_TOKEN,
} TokenType;

typedef struct {
  TokenType type;
  std::string data;
} Token;

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
  void emit(TokenType type, const char data);
  void emit(TokenType type, const std::string data);
  void create(TokenType type, const char data);
  void create(TokenType type, const std::string data);
  void emitCurrent();

  TokenizerState current_state = DATA;
  TokenizerState return_state = UNDEFINED;

  const char *input = nullptr;
  size_t input_size = 0;
  size_t input_ptr = 0;
  char current_char = 0;

  Token current_token;
  std::vector<Token> tokens = {};
};

} // namespace LibHTML
