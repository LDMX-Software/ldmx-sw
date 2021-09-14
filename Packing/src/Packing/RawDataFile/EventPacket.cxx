
#include "Packing/RawDataFile/EventPacket.h"

#include "Packing/Utility/Mask.h"

namespace packing {
namespace rawdatafile {

void EventPacket::read(Reader& r) {
  r >> event_id_;

  uint32_t word;
  r >> word;

  num_subsystems_ = (word >> 16) & utility::mask<16>;
  event_length_in_words_ = (word >> 1) & utility::mask<15>;
  crc_ok_ = word & utility::mask<1>;

  subsys_data_.resize(num_subsystems_);
  for (auto& subsys : subsys_data_) {
    subsys.read(r);
  }

  r >> crc_;
}

/*
void EventPacket::write(std::ostream& os) {
  std::ostreambuf_iterator<uint32_t> wit(os);
  
  wit = event_id_;

  wit++;

}
*/

}
}
