#include "libhtml/exceptions.h"

namespace LibHTML {

StringException::StringException(const char *text) : m_text(text) {}
const char *StringException::what() const throw() { return m_text; }

} // namespace LibHTML
