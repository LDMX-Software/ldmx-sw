
#include "Packing/RawDataFile/EventPacket.h"

#include "Packing/Utility/Mask.h"

namespace packing {
namespace rawdatafile {

void EventPacket::read(Reader& r) {
  r.read(&event_id_, 1);

  uint32_t word;
  r.read(&word, 1);

  num_subsystems_ = (word >> 16) & mask<16>;
  event_length_in_words_ = (word >> 1) & mask<15>;
  crc_ok_ = word & mask<1>;

  subsys_data_.resize(num_subsystems_);
  for (auto& subsys : subsys_data_) {
    subsys.read(r);
  }

  r.read(&crc_, 1);
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
