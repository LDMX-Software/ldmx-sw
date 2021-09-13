#ifndef PACKING_RAWDATAFILE_READER_H_
#define PACKING_RAWDATAFILE_READER_H_

#include <fstream>
#include <string>

#include "Packing/RawDataFile/SubsystemPacket.h"

namespace packing {

/**
 * @namespace rawdatafile
 *
 * Reader and Writer for LDMX raw data file.
 */
namespace rawdatafile {

/**
 * Event Packet structure
 */
class EventPacket {
 public:
  EventPacket() = default;
  ~EventPacket() = default;

  /**
   * read a packet from the input stream
   */
  void read(std::istream& is);

  /// Get subsystem data
  const std::vector<SubsystemPacket>& get() const {
    return subsys_data_;
  }

  /**
   * write a packet to the output stream
   */
  void write(std::ostream& os);

  /// Insert some subsystem data
  void insert(const SubsystemPacket& data) {
    subsys_data_.emplace_back(data);
  }

 private:
  unsigned int event_id_;
  unsigned int num_subsystems_;
  bool crc_ok_;
  unsigned int event_length_in_words_;
  uint32_t crc_read_in_;
  std::vector<SubsystemPacket> subsys_data_;
};  // EventPacket

}  // namespace rawdatafile
}  // namespace packing

/// input streaming operator
std::istream& operator>> (std::istream& is, packing::rawdatafile::EventPacket& ep);

/// output streaming operator
std::ostream& operator<< (std::ostream& os, packing::rawdatafile::EventPacket& ep);

#endif  // PACKING_RAWDATAFILE_READER_H_
