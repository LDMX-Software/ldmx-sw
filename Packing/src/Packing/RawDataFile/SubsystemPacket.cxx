
#include "Packing/RawDataFile/SubsystemPacket.h"

#include "Packing/Utility/Mask.h"

namespace packing {
namespace rawdatafile {

void SubsystemPacket::read(istream& is) {
  static uint32_t word;

  is.read(&word, 4);

  subsys_id_ = (word >> 16) & mask<16>;
  uint32_t subsys_len = (word >> 1) & mask<15>;
  crc_ok_ = word & mask<1>;

  is.seekg(4, std:ios::cur);
  
  data_.reserve(subsys_len);
  is.read(data_.data(), 4*subsys_len);

  is.seekg(4*subsys_len+4, std:ios::cur);
  is.read(&crc_, 4);
}

void SubsystemPacket::write(ostream& os) {
  // static local word to use for writing
  static uint32_t word;
  
  word = ((subsys_id_ & mask<16>) << 16
        +(data_.size() & mask<15>) << 1
        +(crc_ok_ ? 1 : 0));

  os.write(reinterpret_cast<const std::byte*>(&word), 4);

  os.write(
      reinterpret_cast<const std::byte*>(data_.data()),
      data_.size()*4);


  os.write(reinterpret_cast<const std::byte*>(*crc_), 4);
}

}  // namespace rawdatafile
}  // namespace packing

packing::rawdatafile::istream& operator>> (packing::rawdatafile::istream& is, packing::rawdatafile::SubsystemPacket& p) {
  p.read(is);
  return is;
}

packing::rawdatafile::ostream& operator<< (packing::rawdatafile::ostream& os, packing::rawdatafile::SubsystemPacket& p) {
  p.write(os);
  return os;
}
