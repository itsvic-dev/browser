#include "libdom/node.h"
#include <cxxabi.h>
#include <iostream>
#include <memory>

namespace LibDOM {

void Node::appendChild(std::shared_ptr<Node> node) {
  std::wcout << "(dom) appending " << node->internalName().c_str() << " ("
             << node->nodeName << ") to " << this->internalName().c_str()
             << " (" << this->nodeName << ")\n";
  childNodes.push_back(node);
  node->parentNode = this;
}

const std::string Node::internalName() {
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
