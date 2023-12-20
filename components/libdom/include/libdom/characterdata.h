#ifndef LIBDOM_CHARACTERDATA_H
#define LIBDOM_CHARACTERDATA_H

#include "domstring.h"
#include "node.h"

namespace LibDOM {

class CharacterData : public Node {
public:
  DOMString data;
};

} // namespace LibDOM

#endif
