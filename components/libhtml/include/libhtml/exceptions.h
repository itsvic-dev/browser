#ifndef LIBHTML_EXCEPTIONS_H
#define LIBHTML_EXCEPTIONS_H

#include <exception>

namespace LibHTML {

class StringException : public std::exception {
public:
  StringException(const char *text);
  const char *what() const throw();

private:
  const char *m_text;
};

} // namespace LibHTML

#endif
