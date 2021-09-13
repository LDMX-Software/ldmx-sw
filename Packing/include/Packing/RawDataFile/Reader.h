#ifndef PACKING_RAWDATAFILE_READER_H_
#define PACKING_RAWDATAFILE_READER_H_

#include <fstream>
#include <string>

#include "Packing/RawDataFile/EventPacket.h"

namespace packing {

/**
 * @namespace rawdatafile
 *
 * Reader and Writer for LDMX raw data file.
 */
namespace rawdatafile {

/**
 * @class Reader
 * Reading a raw data file.
 */
class Reader {
 public:
  /// open the input file stream and get general info
  Reader(std::string_view file_name);
  /// destructor, close the input file stream
  ~Reader() = default;

  /**
   * Get the next event packet.
   *
   * @return map of subsystem ID to subsystem data
   */
  const EventPacket& next();

 private:
  /// file stream we are reading from
  std::ifstream file_;
  /// number of events in the file
  unsigned int num_events_;
  /// run ID from this file
  unsigned int run_id_;
  /// raw data format version number we are reading
  unsigned short version_;
  /// current event packet
  EventPacket event_;
};  // RawDataFile

}  // namespace rawdatafile
}  // namespace packing

#endif  // PACKING_RAWDATAFILE_READER_H_
