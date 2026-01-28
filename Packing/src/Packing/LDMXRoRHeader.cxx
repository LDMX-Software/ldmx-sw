#include "Packing/LDMXRoRHeader.h"

#include <cassert>

namespace packing {

utility::Reader& LDMXRoRHeader::read(utility::Reader& r) {
  uint8_t sentinel;
  uint32_t zero;
  if (!(r >> version_ >> subsystem_ >> contributor_ >> sentinel)) {
    return r;
  }

  // sentinel should be 0xa5
  assert(sentinel == 0xa5);

  if (!(r >> zero)) {
    return r;
  }

  // next 32b should be zero since they are unused
  assert(zero == 0);

  return (r >> timestamp_);
}

}  // namespace packing
