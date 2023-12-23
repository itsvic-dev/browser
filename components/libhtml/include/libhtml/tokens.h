#ifndef LIBHTML_TOKENS_H
#define LIBHTML_TOKENS_H

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
  virtual ~Token() = default;
  virtual TokenType type();
};

class CharacterToken : public Token {
public:
  CharacterToken(wchar_t c);
  TokenType type();
  wchar_t character();

private:
  wchar_t m_c;
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
  void appendName(wchar_t c);
  void appendName(const std::wstring name);
  void setName(const std::wstring name);

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
  void appendData(wchar_t c);

private:
  std::wstring m_data = L"";
};

class DoctypeToken : public Token {
public:
  DoctypeToken() = default;
  DoctypeToken(wchar_t c);
  DoctypeToken(std::wstring name);
  TokenType type();

  std::wstring name();
  void appendName(wchar_t c);
  void appendName(const std::wstring name);
  bool forceQuirks();
  void setForceQuirks(bool v);

private:
  std::wstring m_name = L"";
  bool m_forceQuirks = false;
};

} // namespace LibHTML

#endif
