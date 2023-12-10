#include "libdom/text.h"
#include "libdom/characterdata.h"
#include "libdom/domstring.h"

namespace LibDOM {

Text::Text(DOMString data) : CharacterData() {
  this->data = data;
  this->nodeType = Node::TEXT_NODE;
}

} // namespace LibDOM
