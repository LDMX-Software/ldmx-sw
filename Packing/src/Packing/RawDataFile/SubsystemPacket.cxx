
#include "Packing/RawDataFile/SubsystemPacket.h"

#include "Packing/Utility/Mask.h"

namespace packing {
namespace rawdatafile {

SubsystemPacket::SubsystemPacket(uint32_t event, uint16_t id, std::vector<uint32_t> data) : event_{event}, id_{id}, data_{data} {
  crc_ok_ = true;

  utility::CRC crc;
  crc << data_;

  crc_ = crc.get();
}

std::vector<uint32_t> SubsystemPacket::header() const {
  uint32_t word = ((id_ & utility::mask<16>) << 16)
        +((data_.size() & utility::mask<15>) << 1)
        +crc_ok_;
  return { word, event_ };
}

std::vector<uint32_t> SubsystemPacket::tail() const {
  return { crc_ };
}

utility::Reader& SubsystemPacket::read(utility::Reader& r) {
  uint32_t word;
  r >> word;

  id_ = (word >> 16) & utility::mask<16>;
  uint32_t len = (word >> 1) & utility::mask<15>;
  crc_ok_ = word & utility::mask<1>;

  r >> event_;

  r.read(data_, len);

  r >> crc_;

  return r;
}

utility::Writer& SubsystemPacket::write(utility::Writer& w) const {
  std::vector<uint32_t> head{header()}, t{tail()};
  w << head << data_ << t;
  return w;
}

utility::CRC& SubsystemPacket::add(utility::CRC& c) const {
  std::vector<uint32_t> head{header()}, t{tail()};
  c << head << data_ << t;
  return c;
}

}  // namespace rawdatafile
}  // namespace packing
