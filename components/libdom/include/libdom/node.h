#ifndef LIBDOM_NODE_H
#define LIBDOM_NODE_H

#include "libdom/domstring.h"
#include <memory>
#include <vector>

namespace LibDOM {

class Document;

class Node {
public:
  virtual ~Node() = default;

  static const unsigned short ELEMENT_NODE = 1;
  static const unsigned short ATTRIBUTE_NODE = 2;
  static const unsigned short TEXT_NODE = 3;
  static const unsigned short CDATA_SECTION_NODE = 4;
  static const unsigned short ENTITY_REFERENCE_NODE = 5; // legacy
  static const unsigned short ENTITY_NODE = 6;           // legacy
  static const unsigned short PROCESSING_INSTRUCTION_NODE = 7;
  static const unsigned short COMMENT_NODE = 8;
  static const unsigned short DOCUMENT_NODE = 9;
  static const unsigned short DOCUMENT_TYPE_NODE = 10;
  static const unsigned short DOCUMENT_FRAGMENT_NODE = 11;
  static const unsigned short NOTATION_NODE = 12; // legacy

  unsigned short nodeType;
  DOMString nodeName;
  std::shared_ptr<Document> ownerDocument = nullptr;
  Node *parentNode = nullptr;
  std::vector<std::shared_ptr<Node>> childNodes;

  virtual void appendChild(std::shared_ptr<Node> node);

  virtual const std::string internalName();
};

} // namespace LibDOM

#endif
