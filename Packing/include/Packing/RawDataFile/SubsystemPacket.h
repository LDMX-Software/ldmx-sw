#ifndef PACKING_RAWDATAFILE_SUBSYSTEMPACKET_H_
#define PACKING_RAWDATAFILE_SUBSYSTEMPACKET_H_

#include <vector>

#include "Packing/Utility/Reader.h"
#include "Packing/Utility/Writer.h"
#include "Packing/Utility/CRC.h"

namespace packing {
namespace rawdatafile {

/**
 * SubsystemPacket structure
 */
class SubsystemPacket {
 public:
  /// default constructor for reading
  SubsystemPacket() = default;

  /// constructor for writing
  SubsystemPacket(uint32_t event, uint16_t id, std::vector<uint32_t> data);

  /**
   * get the header words
   */
  std::vector<uint32_t> header() const;

  /**
   * Get data
   */
  const std::vector<uint32_t>& data() const {
    return data_;
  }
  const uint16_t& id() const { return id_; }

  /**
   * get the tailing words
   */
  std::vector<uint32_t> tail() const;

  /**
   * read the subsystem packet from the input reader
   */
  utility::Reader& read(utility::Reader& r);
  /**
   * write the subsystem packet to the input writer
   */
  utility::Writer& write(utility::Writer& w) const;

  /**
   * add the subsystem packet to the input crc
   */
  utility::CRC& add(utility::CRC& c) const;

 private:
  uint32_t event_;
  uint16_t id_;
  std::vector<uint32_t> data_;
  bool crc_ok_;
  unsigned int crc_;
};  // SubsystemPacket

}  // namespace rawdatafile
}  // namespace packing

#endif  // PACKING_RAWDATAFILE_SUBSYSTEMPACKET_H_
