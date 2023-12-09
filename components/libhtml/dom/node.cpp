#include "libhtml/dom/node.h"
#include <iostream>

namespace LibHTML::DOM {

void Node::appendChild(std::shared_ptr<Node> node) {
  std::cout << "(dom) appending " << node << " to " << this << "\n";
  childNodes.push_back(node);
}

} // namespace LibHTML::DOM
