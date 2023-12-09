#pragma once
#include <string>
#include <vector>

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

typedef struct {
  std::wstring name;
  std::wstring value;
} Attribute;

class TagToken : public Token {
public:
  TagToken(TokenType type);
  TokenType type();

  std::wstring name();
  void appendName(char c);
  void appendName(const std::wstring name);

  bool selfClosing = false;
  std::vector<Attribute> attributes = {};

private:
  std::wstring m_name = L"";
  TokenType m_type;
};

class CommentToken : public Token {
public:
  CommentToken() = default;
  CommentToken(std::wstring data);
  TokenType type();

  std::wstring data();
  void appendData(char c);

private:
  std::wstring m_data = L"";
};

class DoctypeToken : public Token {
public:
  DoctypeToken() = default;
  DoctypeToken(char c);
  DoctypeToken(std::wstring name);
  TokenType type();

  std::wstring name();
  void appendName(char c);
  void appendName(const std::wstring name);
  bool forceQuirks();
  void setForceQuirks(bool v);

private:
  std::wstring m_name = L"";
  bool m_forceQuirks = false;
};

} // namespace LibHTML
