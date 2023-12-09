#pragma once
#include "libhtml/dom/domstring.h"
#include <memory>
#include <vector>

namespace LibHTML::DOM {

class Document;

class Node {
public:
  static const unsigned short ELEMENT_MODE = 1;
  static const unsigned short ATTRIBUTE_NODE = 2;
  static const unsigned short TEXT_NODE = 3;
  static const unsigned short CDATA_SECTION_NODE = 4;
  static const unsigned short ENTITY_REFERENCE_MODE = 5; // legacy
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

  void appendChild(std::shared_ptr<Node> node);

  virtual const std::string _name();
};

} // namespace LibHTML::DOM