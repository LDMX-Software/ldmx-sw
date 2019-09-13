#include "Event/ProductTag.h"


std::ostream& operator<<(std::ostream& s,const ldmx::ProductTag& pt) {
  return s << pt.name() << "_" << pt.passname() << "_" << pt.type();
}
