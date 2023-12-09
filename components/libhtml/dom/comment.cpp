#include "libhtml/dom/comment.h"
#include "libhtml/dom/domstring.h"

namespace LibHTML::DOM {

DOMString CharacterData::data() { return L"undefined"; }

Comment::Comment(DOMString data) : m_data(data) {}
DOMString Comment::data() { return m_data; };

} // namespace LibHTML::DOM
