#ifndef LIBDOM_ELEMENT_H
#define LIBDOM_ELEMENT_H

#include "libdom/domstring.h"
#include "libdom/namednodemap.h"
#include "node.h"
#include <memory>

namespace LibDOM {

class Element : public Node {
public:
  Element();

  DOMString namespaceURI;
  DOMString prefix;
  DOMString localName;
  DOMString tagName;

  NamedNodeMap attributes;
  DOMString getAttribute(DOMString qualifiedName);
  void setAttribute(DOMString qualifiedName, DOMString value);
  void removeAttribute(DOMString qualifiedName);
  bool hasAttribute(DOMString qualifiedName);
};

class HTMLElement : public Element {};

class HTMLHtmlElement : public HTMLElement {};
class HTMLHeadElement : public HTMLElement {};

}; // namespace LibDOM

#endif
