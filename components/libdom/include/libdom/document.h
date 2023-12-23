#ifndef LIBDOM_DOCUMENT_H
#define LIBDOM_DOCUMENT_H

#include "libdom/element.h"
#include <memory>

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

  std::shared_ptr<Element> head;
  std::shared_ptr<Element> body;

  bool parserCannotChangeMode = false;
};

} // namespace LibDOM

#endif // LIBDOM_DOCUMENT_H
