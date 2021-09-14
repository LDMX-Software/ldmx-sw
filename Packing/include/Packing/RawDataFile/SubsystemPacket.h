#ifndef PACKING_RAWDATAFILE_SUBSYSTEMPACKET_H_
#define PACKING_RAWDATAFILE_SUBSYSTEMPACKET_H_

#include <vector>

#include "Packing/RawDataFile/Stream.h"

namespace packing {
namespace rawdatafile {

/**
 * SubsystemPacket structure
 */
class SubsystemPacket {
 public:
  SubsystemPacket() = default;
  ~SubsystemPacket() = default;

  /**
   * read a packet form the input stream
   */
  void read(Reader& r);

  /**
   * write a packet to the output stream
  void write(std::ofstream& os);
   */

  /**
   * Get data
   */
  const std::vector<WordType>& get() const {
    return data_;
  }

  /// insert data
  void insert(const std::vector<WordType>& data);

 private:
  bool crc_ok_;
  unsigned int subsys_id_;
  std::vector<WordType> data_;
  unsigned int crc_;
};  // SubsystemPacket

}  // namespace rawdatafile
}  // namespace packing

#endif  // PACKING_RAWDATAFILE_SUBSYSTEMPACKET_H_
