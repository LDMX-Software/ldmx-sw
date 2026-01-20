#include "Packing/RogueFrame.h"

namespace packing {

utility::Reader& RogueFrame::read(utility::Reader& r) {
  if (!(r >> size_ >> flags_ >> error_ >> channel_)) {
    return r;
  }

  // subtract 4 from size since 4 bytes after
  // the size bytes have been read
  size_ -= 4;

  return r.read(data_, size_);
}

}
