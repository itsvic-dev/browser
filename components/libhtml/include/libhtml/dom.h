#pragma once
#include <string>

namespace LibHTML::DOM {

typedef std::wstring DOMString;

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
};

class CharacterData : public Node {
public:
  DOMString data;
};

class Comment : public CharacterData {
public:
  Comment(DOMString data = L"");
};

class DocumentType : public Node {
public:
  DOMString name;
  DOMString publicId;
  DOMString systemId;
};

class Document : public Node {
public:
  std::string mode = "no-quirks";
};

} // namespace LibHTML::DOM
