#include "libdom/comment.h"
#include "libdom/domstring.h"

namespace LibDOM {

Comment::Comment(DOMString data) : CharacterData() { this->data = data; }

} // namespace LibDOM
