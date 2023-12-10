#include "libdom/comment.h"
#include "libdom/domstring.h"

namespace LibDOM {

DOMString CharacterData::data() { return L"undefined"; }

Comment::Comment(DOMString data) : m_data(data) {}
DOMString Comment::data() { return m_data; };

} // namespace LibDOM
