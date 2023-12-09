#pragma once
#include "libhtml/dom/domstring.h"
#include "node.h"
#include <memory>

namespace LibHTML::DOM {

class Element;

class Attr : public Node {
public:
  DOMString namespaceURI;
  DOMString prefix;
  DOMString localName;
  DOMString name;
  DOMString value;

  Element *ownerElement = nullptr;
  const bool specified = true;
};

class Element : public Node {
public:
  DOMString namespaceURI;
  DOMString prefix;
  DOMString localName;
  DOMString tagName;
};

class HTMLElement : public Element {};

}; // namespace LibHTML::DOM
