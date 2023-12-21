#include "libhtml/parser.h"
#include <iostream>
#include <string>

int main() {
  LibHTML::Parser parser;

  std::string s(std::istreambuf_iterator<char>(std::cin), {});
  parser.parse(s.c_str(), s.size());

  // let the parser know we're EOF'd now
  const wchar_t eof[] = {EOF};
  parser.parse(eof, 1);

  return 0;
}
