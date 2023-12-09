#pragma once
#include "domstring.h"
#include "node.h"

namespace LibHTML::DOM {

class CharacterData : public Node {
public:
  virtual DOMString data();
};

class Comment : public CharacterData {
public:
  Comment(DOMString data = L"");
  DOMString data();

private:
  DOMString m_data;
};

} // namespace LibHTML::DOM
