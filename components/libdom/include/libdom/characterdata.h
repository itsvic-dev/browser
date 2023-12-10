#pragma once
#include "domstring.h"
#include "node.h"

namespace LibDOM {

class CharacterData : public Node {
public:
  DOMString data;
};

} // namespace LibDOM
