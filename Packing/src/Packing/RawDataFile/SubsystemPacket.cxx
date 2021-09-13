
#include "Packing/RawDataFile/SubsystemPacket.h"

namespace packing {
namespace rawdatafile {

SubsystemPacket::read(std::istream& is) {
  // local read iterator
  std::istream_buf_iterator<uint32_t> rit(is);

  subsys_id_ = (*rit >> 16) & mask<16>;
  uint32_t subsys_len = (*rit >> 1) & mask<15>;
  crc_ok_ = *rit & mask<1>;
  
  rit++;

  data_.reserve(subsys_len);
  data_.insert(data_.begin(), rit, rit+subsys_len);

  crc_ = *rit;
}

SubsystemPacket::write(std::ostream& os) {
  std::ostream_bufiterator<uint32_t> wit(os);
  
  wit = ((subsys_id_ & mask<16>) << 16
        +(data_.size() & mask<15>) << 1
        +(crc_ok_ ? 1 : 0));

  wit++;

  std::copy(data_.begin(), data_.end(), wit);

  wit++;

  wit = crc_;
}

}  // namespace rawdatafile
}  // namespace packing

std::istream& operator>> (std::istream& is, packing::rawdatafile::SubsystemPacket& p) {
  p.read(is);
  return is;
}

std::ostream& operator<< (std::ostream& os, packing::rawdatafile::SubsystemPacket& p) {
  p.write(os);
  return os;
}
