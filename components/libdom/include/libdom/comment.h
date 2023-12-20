#ifndef LIBDOM_COMMENT_H
#define LIBDOM_COMMENT_H

#include "characterdata.h"
#include "domstring.h"
#include "node.h"

namespace LibDOM {

class Comment : public CharacterData {
public:
  Comment(DOMString data = L"");
};

} // namespace LibDOM

#endif
