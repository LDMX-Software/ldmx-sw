#include "Packing/RogueFrameHeader.h"

namespace packing {

utility::Reader& RogueFrameHeader::read(utility::Reader& r) {
  if (!(r >> size_ >> flags_ >> error_ >> channel_)) {
    return r;
  }

  // subtract 4 from size since 4 bytes after
  // the bytes holding size have been read
  size_ -= 4;

  return r;
}

}
