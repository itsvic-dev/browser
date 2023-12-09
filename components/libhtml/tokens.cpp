#include "libhtml/tokens.h"
#include <string>

namespace LibHTML {

TokenType Token::type() { return UNDEFINED_TOKEN; }

CharacterToken::CharacterToken(char c) : c(c) {}
TokenType CharacterToken::type() { return CHARACTER; }
char CharacterToken::character() { return c; }

TokenType EOFToken::type() { return END_OF_FILE; }

TagToken::TagToken(TokenType type) : m_type(type) {}
TokenType TagToken::type() { return m_type; }
std::string TagToken::name() { return m_name; }
void TagToken::appendName(char c) { m_name += c; }
void TagToken::appendName(const std::string name) { m_name += name; }

CommentToken::CommentToken(std::string data) : m_data(data) {}
TokenType CommentToken::type() { return COMMENT; }
std::string CommentToken::data() { return m_data; }
void CommentToken::appendData(char c) { m_data += c; }

DoctypeToken::DoctypeToken(char c) : m_name(std::string(1, c)) {}
DoctypeToken::DoctypeToken(std::string name) : m_name(name) {}
TokenType DoctypeToken::type() { return DOCTYPE_TOKEN; }
std::string DoctypeToken::name() { return m_name; }
void DoctypeToken::appendName(char c) { m_name += c; }
void DoctypeToken::appendName(const std::string name) { m_name += name; }
bool DoctypeToken::forceQuirks() { return m_forceQuirks; }
void DoctypeToken::setForceQuirks(bool v) { m_forceQuirks = v; }

} // namespace LibHTML
