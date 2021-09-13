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
  void read(istream& is);

  /**
   * write a packet to the output stream
   */
  void write(ostream& os);

  /**
   * Get data
   */
  const std::vector<unsigned int>& get() const {
    return data_;
  }

  /// insert data
  void insert(const std::vector<std::byte>& data);

 private:
  bool crc_ok_;
  unsigned int subsys_id_;
  std::vector<unsigned int> data_;
  unsigned int crc_;
};  // SubsystemPacket

}  // namespace rawdatafile
}  // namespace packing

/// input streaming operator
packing::rawdatafile::istream& operator>> (packing::rawdatafile::istream& is, packing::rawdatafile::SubsystemPacket& p);

/// output streaming operator
packing::rawdatafile::ostream& operator<< (packing::rawdatafile::ostream& os, packing::rawdatafile::SubsystemPacket& p);

#endif  // PACKING_RAWDATAFILE_SUBSYSTEMPACKET_H_
