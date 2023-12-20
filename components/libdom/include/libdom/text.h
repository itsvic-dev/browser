#ifndef LIBDOM_TEXT_H
#define LIBDOM_TEXT_H

#include "characterdata.h"
#include "domstring.h"
#include "node.h"

namespace LibDOM {

class Text : public CharacterData {
public:
  Text(DOMString data = L"");
};

} // namespace LibDOM

#endif
