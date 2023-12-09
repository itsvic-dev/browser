#pragma once
#include "libhtml/dom/domstring.h"
#include "libhtml/dom/node.h"
#include <string>

namespace LibHTML::DOM {

class DocumentType : public Node {
public:
  DOMString name;
  DOMString publicId;
  DOMString systemId;
};

class Document : public Node {
public:
  std::string mode = "no-quirks";
  bool parserCannotChangeMode = false;
};

} // namespace LibHTML::DOM
