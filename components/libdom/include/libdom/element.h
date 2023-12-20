#ifndef LIBDOM_ELEMENT_H
#define LIBDOM_ELEMENT_H

#include "libdom/domstring.h"
#include "node.h"
#include <memory>

namespace LibDOM {

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
  Element();

  DOMString namespaceURI;
  DOMString prefix;
  DOMString localName;
  DOMString tagName;
};

class HTMLElement : public Element {};

class HTMLHtmlElement : public HTMLElement {};
class HTMLHeadElement : public HTMLElement {};

}; // namespace LibDOM

#endif
