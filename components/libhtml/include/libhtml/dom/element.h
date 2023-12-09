#pragma once
#include "libhtml/dom/domstring.h"
#include "node.h"

namespace LibHTML::DOM {

class Element : public Node {
public:
  DOMString namespaceURI;
  DOMString prefix;
  DOMString localName;
  DOMString tagName;
};

class HTMLElement : public Element {};

}; // namespace LibHTML::DOM
