#include "libdom/namednodemap.h"
namespace LibDOM {

unsigned long NamedNodeMap::length() { return m_attrs.size(); }
std::shared_ptr<Attr> NamedNodeMap::item(unsigned long index) {
  return m_attrs[index];
}

std::shared_ptr<Attr> NamedNodeMap::setNamedItem(std::shared_ptr<Attr> attr) {
  m_attrs.push_back(attr);
  return attr;
}

} // namespace LibDOM
