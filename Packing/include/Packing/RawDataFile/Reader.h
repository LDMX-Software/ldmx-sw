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
 * @class Reader
 * Reading a raw data file.
 *
 * We wrap a basic std::ifstream in order to make the retrieving
 * of specific-width words easier for ourselves. Each reader has
 * a defined 'WordType' which makes us able to read-out data files,
 * getting words one or more at a time.
 */
class Reader {
 public:
  /// open the input file stream and get general info
  Reader(std::string_view file_name) : file_{file_name, std::ios::binary} {
    file_.unsetf(std::ios::skipws);
  }

  /// destructor, close the input file stream
  ~Reader() = default;

  /**
   * Go ("seek") a specific position in the file.
   */
  void seek(int off, std::ios_base::seekdir dir = std::ios::beg) {
    file_.seekg(off*word_type_size_, dir);
  }

  /**
   * Tell us where the reader is
   */
  int tell() const { return file_.tellg()/word_type_size_; }

  /**
   * Read the next 'count' words into the input handle.
   */
  template <typename PointerType>
  void read(PointerType* dest, int count) {
    file_.read(reinterpret_cast<char*>(dest), word_type_size_*count);
    file_.seekg(word_type_size_*count, std::ios::cur);
  }

 private:
  /// the word type we are using internally
  typedef uint32_t WordType;
  /// size of our WordType
  static const std::size_t word_type_size_{sizeof(WordType)};

 private:
  /// file stream we are reading from
  std::ifstream file_;
};  // RawDataFile

}  // namespace rawdatafile
}  // namespace packing

#endif  // PACKING_RAWDATAFILE_READER_H_
