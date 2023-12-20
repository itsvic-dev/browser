#ifndef LIBDOM_NAMEDNODEMAP_H
#define LIBDOM_NAMEDNODEMAP_H

#include "libdom/domstring.h"
#include "libdom/node.h"
#include <memory>
#include <vector>

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

class NamedNodeMap {
public:
  unsigned long length();
  std::shared_ptr<Attr> item(unsigned long index);
  std::shared_ptr<Attr> getNamedItem(DOMString qualifiedName);
  std::shared_ptr<Attr> setNamedItem(std::shared_ptr<Attr> attr);
  std::shared_ptr<Attr> removeNamedItem(DOMString qualifiedName);

private:
  std::vector<std::shared_ptr<Attr>> m_attrs;
};

}; // namespace LibDOM

#endif
