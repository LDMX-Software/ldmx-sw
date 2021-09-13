
#include "Packing/RawDataFile/EventPacket.h"

#include "Packing/Utility/Mask.h"

namespace packing {
namespace rawdatafile {

void EventPacket::read(std::istream& is) {
  // local read iterator
  std::istreambuf_iterator<uint32_t> rit(is);

  event_id_ = *rit;

  rit++;

  num_subsystems_ = (*rit >> 16) & mask<16>;
  event_length_in_words_ = (*rit >> 1) & mask<15>;
  crc_ok_ = *rit & mask<1>;

  auto end = rit + event_length_in_words_;

  for (unsigned i_subsys{0}; i_subsys < num_subsystems_; i_subsys++) {
    rit++;

    uint32_t subsys_id = (*rit >> 16) & mask<16>;
    uint32_t subsys_len = (*rit >> 1) & mask<15>;
    bool subsys_crc_ok = *rit & mask<1>;
    
    rit++;

    std::vector<uint32_t> subsys_data{rit,rit+subsys_len};

    uint32_t subsys_crc = *rit;
  }

  rit++;

  uint32_t crc_read_in_ = *rit;
}

void EventPacket::write(std::ostream& os) {
  std::ostreambuf_iterator<uint32_t> wit(os);
  
  wit = event_id_;

  wit++;

}

}
}

/// input streaming operator
std::istream& operator>> (std::istream& is, packing::rawdatafile::EventPacket& ep) {
  ep.read(is);
  return is;
}

/// output streaming operator
std::ostream& operator<< (std::ostream& os, packing::rawdatafile::EventPacket& ep) {
  ep.write(os);
  return os;
}
