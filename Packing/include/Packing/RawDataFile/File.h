#ifndef PACKING_RAWDATAFILE_FILE_H_
#define PACKING_RAWDATAFILE_FILE_H_

#include "Packing/RawDataFile/EventPacket.h"
#include "Packing/Utility/Reader.h"
#include "Packing/Utility/Writer.h"

namespace packing {
namespace rawdatafile {

/**
 * The raw data file object
 */
class File {
 public:
  /**
   * General file constructor
   *
   * @param[in] params parameters use to configure this file
   * @param[in] filename name of the file to read/write
   */
  File(const framework::config::Parameters &params);

  /**
   * Connect the passed event bus to this event file.
   */
  bool connect(framework::Event& event);

  /**
   * Load the next event into our connected event bus.
   */
  bool nextEvent();

  /**
   * Write the run header
   */
  void writeRunHeader(ldmx::RunHeader &header);

  /// close this file
  void close();

 private:
  /// number of entries in the file
  long int entries_{-1};
  /// current entry index (may not be same as event number)
  long int i_entry_{-1};
  /// are we reading or writing?
  bool is_output_;
  /// handle to the event bus we are reading from or writing to
  framework::Event* event_{nullptr};
  /// run number corresponding to this file of raw data
  uint32_t run_;

 private:
  /// utility class for reading binary data files
  utility::Reader reader_;
  /// utility class for writing binary data files
  utility::Writer writer_;
};  // File

}
}

#endif  // PACKING_RAWDATAFILE_FILE_H_
