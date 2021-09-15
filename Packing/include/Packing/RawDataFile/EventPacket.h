#ifndef PACKING_RAWDATAFILE_EVENTPACKET_H_
#define PACKING_RAWDATAFILE_EVENTPACKET_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "Packing/RawDataFile/SubsystemPacket.h"

namespace packing {
namespace rawdatafile {

/**
 * Event Packet structure
 */
class EventPacket {
 public:
  /// default constructor for reading
  EventPacket() = default;

  /// define event id for reading
  EventPacket(uint32_t id, const std::map<uint16_t,std::vector<uint32_t>>& unwrapped_subsys_data);

  /// Get the header words
  std::vector<uint32_t> header() const;

  /// Get the tail words
  std::vector<uint32_t> tail() const;

  const uint32_t& id() const { return id_; }
  const std::vector<SubsystemPacket>& data() const { return subsys_data_; }

  /**
   * read the event packet from the input reader
   */
  utility::Reader& read(utility::Reader& r);
  /**
   * write the event packet to the input writer
   */
  utility::Writer& write(utility::Writer& w) const;

  /**
   * add the event packet to the input crc
   */
  utility::CRC& add(utility::CRC& c) const;

 private:
  uint32_t id_;
  uint16_t event_length_in_words_;
  std::vector<SubsystemPacket> subsys_data_;
  bool crc_ok_;
  uint32_t crc_;
};  // EventPacket

}  // namespace rawdatafile
}  // namespace packing

#endif  // PACKING_RAWDATAFILE_EVENTPACKET_H_
