#include "libdom/element.h"
#include "libdom/domstring.h"
#include "libdom/namednodemap.h"
#include <memory>

namespace LibDOM {

Element::Element() { this->nodeType = Node::ELEMENT_NODE; }

DOMString Element::getAttribute(DOMString qualifiedName) {
  auto a = attributes.getNamedItem(qualifiedName);
  if (a == nullptr)
    return L"";
  return a->value;
}

void Element::setAttribute(DOMString qualifiedName, DOMString value) {
  auto attr = std::make_shared<Attr>();
  attr->name = qualifiedName;
  attr->value = value;
  attributes.setNamedItem(attr);
}

bool Element::hasAttribute(DOMString qualifiedName) {
  return attributes.getNamedItem(qualifiedName) != nullptr;
}

} // namespace LibDOM
