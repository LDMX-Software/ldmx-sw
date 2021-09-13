
#include "Packing/RawDataFile/Reader.h"

namespace packing {
namespace rawdatafile {

Reader::Reader(std::string_view file_name) : file_{file_name, std::ios::binary} {

  uint32_t word;
  // start from beginning
  file_.seekg(0);
  // read 4 bytes into header word
  file_.read(word, 4);

  version_ = (word >> 28) & mask<4>;
  run_id_  = word & mask<28>;

  // currently only version 0 is implemented
  if (version_ != 0 ) {
    EXCEPTION_RAISE("RawFileVers",
        "RawFile Format version "+std::to_string(version)
        + " is not implemented at this time.");
  }

  // go to the end of the file and retrieve the number of events in the file
  file_.seekg(-2*4, std::ios::end); // -8 bytes (2 32-bit words) is the beginning of the 2nd to last word
  file_.read(word, 4);

  num_events_ = word;

  // reset the current position to the beginning of the first event packet
  file_.seekg(1*4, std::ios::beg);
}

const EventPacket& Reader::next() {
  file_ >> event_;
  return event_;
}

}
}
