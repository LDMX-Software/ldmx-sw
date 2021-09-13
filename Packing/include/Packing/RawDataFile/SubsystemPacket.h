#ifndef PACKING_RAWDATAFILE_READER_H_
#define PACKING_RAWDATAFILE_READER_H_

#include <fstream>
#include <string>

namespace packing {

/**
 * @namespace rawdatafile
 *
 * Reader and Writer for LDMX raw data file.
 */
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
  void read(std::istream& is);

  /**
   * write a packet to the output stream
   */
  void write(std::ostream& os);

  /**
   * Get data
   */
  const std::vector<unsigned int>& get() const {
    return data_;
  }

  /// insert data
  void insert(const std::vector<unsigned int>& data);

 private:
  bool crc_ok_;
  unsigned int subsys_id_;
  std::vector<unsigned int> data_;
  unsigned int crc_;
};  // SubsystemPacket

}  // namespace rawdatafile
}  // namespace packing

/// input streaming operator
std::istream& operator>> (std::istream& is, packing::rawdatafile::SubsystemPacket& p);

/// output streaming operator
std::ostream& operator<< (std::ostream& os, packing::rawdatafile::SubsystemPacket& p);

#endif  // PACKING_RAWDATAFILE_READER_H_
