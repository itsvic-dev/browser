#include "libhtml/ast.h"
#include <iostream>

namespace LibHTML {

void ASTParser::parse(std::vector<std::shared_ptr<Token>> tokens) {
  while (tokenPtr < tokens.size()) {
    parseTick();
  }
}

void ASTParser::parseTick() {
  switch (insertionMode) {
  default:
    std::cerr << "[LibHTML] Unhandled insertion mode " << insertionMode << "\n";
    throw 0;
  }
}

} // namespace LibHTML
