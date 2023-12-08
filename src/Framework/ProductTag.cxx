#include "Framework/ProductTag.h"

std::ostream& operator<<(std::ostream& s, const framework::ProductTag& pt) {
  return s << "{ name = " << pt.name() << ", pass = " << pt.passname() << ", type = " << pt.type() << "}";
}
