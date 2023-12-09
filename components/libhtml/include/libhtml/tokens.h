#pragma once
#include <string>

namespace LibHTML {

enum TokenType : int {
  UNDEFINED_TOKEN,
  CHARACTER,
  END_OF_FILE,
  START_TAG,
  END_TAG,
  COMMENT,
  DOCTYPE_TOKEN,
};

class Token {
public:
  Token() = default;
  virtual TokenType type();
};

class CharacterToken : public Token {
public:
  CharacterToken(char c);
  TokenType type();
  char character();

private:
  char c;
};

class EOFToken : public Token {
public:
  EOFToken() = default;
  TokenType type();
};

class TagToken : public Token {
public:
  TagToken(TokenType type);
  TokenType type();

  std::string name();
  void appendName(char c);
  void appendName(const std::string name);

private:
  std::string m_name = "";
  TokenType m_type;
};

class CommentToken : public Token {
public:
  CommentToken() = default;
  CommentToken(std::string data);
  TokenType type();

  std::string data();
  void appendData(char c);

private:
  std::string m_data = "";
};

class DoctypeToken : public Token {
public:
  DoctypeToken() = default;
  DoctypeToken(char c);
  DoctypeToken(std::string name);
  TokenType type();

  std::string name();
  void appendName(char c);
  void appendName(const std::string name);
  bool forceQuirks();
  void setForceQuirks(bool v);

private:
  std::string m_name = "";
  bool m_forceQuirks = false;
};

} // namespace LibHTML
