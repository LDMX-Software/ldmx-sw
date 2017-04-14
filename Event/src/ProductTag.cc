#include "Event/ProductTag.h"

namespace ldmx {

  std::ostream& operator<<(std::ostream& s,const ldmx::ProductTag& pt) {

    return s << pt.name() << "::" << pt.passname() << "::" << pt.type();

  }
}
