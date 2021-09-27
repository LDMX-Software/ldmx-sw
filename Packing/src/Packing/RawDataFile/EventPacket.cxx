
#include "Packing/RawDataFile/EventPacket.h"

#include "Packing/Utility/Mask.h"

namespace packing {
namespace rawdatafile {

EventPacket::EventPacket(uint32_t id, const std::map<uint16_t,std::vector<uint32_t>>& unwrapped_subsys_data) 
  : id_{id}, event_length_in_words_{0} {
  crc_ok_ = true;

  utility::CRC crc;

  for (auto const&[subsys_id, subsys_data] : unwrapped_subsys_data) {
    subsys_data_.emplace_back(id_,subsys_id,subsys_data);
    auto& subsys = subsys_data_.back();
    crc << subsys;
    event_length_in_words_ += subsys.header().size()+subsys.data().size()+subsys.tail().size();
  }

  crc_ = crc.get();
}

std::vector<uint32_t> EventPacket::header() const {
  uint32_t word = ((subsys_data_.size() & utility::mask<16>) << 16)
    + ((event_length_in_words_ & utility::mask<15>) << 1)
    + crc_ok_;
  return { id_ , word };
}

std::vector<uint32_t> EventPacket::tail() const {
  return { crc_ };
}

utility::Reader& EventPacket::read(utility::Reader& r) {
  r >> id_;

  uint32_t word;
  r >> word;

  uint16_t num_subsys = (word >> 16) & utility::mask<16>;
  event_length_in_words_ = (word >> 1) & utility::mask<15>;
  crc_ok_ = word & utility::mask<1>;

  r.read(subsys_data_, num_subsys);

  r >> crc_;
}

utility::Writer& EventPacket::write(utility::Writer& w) const {
  std::vector<uint32_t> h{header()},t{tail()};
  w << h << subsys_data_ << t;
  return w;
}

}
}
