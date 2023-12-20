#include "libdom/namednodemap.h"
#include <algorithm>
#include <iterator>
namespace LibDOM {

unsigned long NamedNodeMap::length() { return m_attrs.size(); }
std::shared_ptr<Attr> NamedNodeMap::item(unsigned long index) {
  return m_attrs[index];
}

std::shared_ptr<Attr> NamedNodeMap::setNamedItem(std::shared_ptr<Attr> attr) {
  std::shared_ptr<Attr> ret;
  auto it = std::find_if(
      m_attrs.begin(), m_attrs.end(),
      [attr](std::shared_ptr<Attr> a1) { return a1->name == attr->name; });
  if (it == m_attrs.end()) {
    ret = attr;
    m_attrs.push_back(attr);
  } else {
    ret = *it;
    ret->value = attr->value;
  }
  return ret;
}

} // namespace LibDOM
