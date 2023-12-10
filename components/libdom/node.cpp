#include "libdom/node.h"
#include <cxxabi.h>
#include <iostream>
#include <memory>

namespace LibDOM {

void Node::appendChild(std::shared_ptr<Node> node) {
  std::cout << "(dom) appending " << node << " to " << this << "\n";
  childNodes.push_back(node);
  node->parentNode = this;
}

const std::string Node::_name() {
  // get a pretty name of this class
  int status = -1;
  const char *name = typeid(*this).name();
  std::unique_ptr<char, void (*)(void *)> res{
      abi::__cxa_demangle(name, NULL, NULL, &status),
      std::free,
  };
  return (status == 0) ? res.get() : name;
};

} // namespace LibDOM
