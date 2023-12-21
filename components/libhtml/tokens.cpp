#include "libhtml/tokens.h"
#include <string>

namespace LibHTML {

TokenType Token::type() { return UNDEFINED_TOKEN; }

CharacterToken::CharacterToken(wchar_t c) : m_c(c) {}
TokenType CharacterToken::type() { return CHARACTER; }
wchar_t CharacterToken::character() { return m_c; }

TokenType EOFToken::type() { return END_OF_FILE; }

TagToken::TagToken(TokenType type) : m_type(type) {}
TokenType TagToken::type() { return m_type; }
std::wstring TagToken::name() { return m_name; }
void TagToken::appendName(char c) { m_name += c; }
void TagToken::appendName(const std::wstring name) { m_name += name; }
void TagToken::setName(const std::wstring name) { m_name += name; }

CommentToken::CommentToken(std::wstring data) : m_data(data) {}
TokenType CommentToken::type() { return COMMENT; }
std::wstring CommentToken::data() { return m_data; }
void CommentToken::appendData(char c) { m_data += c; }

DoctypeToken::DoctypeToken(char c) : m_name(std::wstring(1, c)) {}
DoctypeToken::DoctypeToken(std::wstring name) : m_name(name) {}
TokenType DoctypeToken::type() { return DOCTYPE_TOKEN; }
std::wstring DoctypeToken::name() { return m_name; }
void DoctypeToken::appendName(char c) { m_name += c; }
void DoctypeToken::appendName(const std::wstring name) { m_name += name; }
bool DoctypeToken::forceQuirks() { return m_forceQuirks; }
void DoctypeToken::setForceQuirks(bool v) { m_forceQuirks = v; }

} // namespace LibHTML
