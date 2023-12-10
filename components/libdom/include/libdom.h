#pragma once
#include "libdom/domstring.h"
#include "libdom/node.h"
#include "libdom/text.h"
#include <string>

namespace LibDOM {

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

} // namespace LibDOM
