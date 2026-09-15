#include "Packing/RogueFrameHeader.h"

namespace packing {

utility::Reader& RogueFrameHeader::read(utility::Reader& r) {
  if (!(r >> size_ >> flags_ >> error_ >> channel_)) {
    return r;
  }

  // subtract 4 from size since 4 bytes after
  // the bytes holding size have been read
  size_ -= 4;

  std::streampos frame_start = r.tell();
  r.seek(frame_start + static_cast<std::streamoff>(size_ - 1));
  r >> trailer_;
  // reset back to beginning of frame
  r.seek(frame_start);

  return r;
}

}  // namespace packing
