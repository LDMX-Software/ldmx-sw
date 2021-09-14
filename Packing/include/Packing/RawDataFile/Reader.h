#ifndef PACKING_RAWDATAFILE_READER_H_
#define PACKING_RAWDATAFILE_READER_H_

#include <fstream>
#include <string>
#include <type_traits>

namespace packing {
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
  Reader(const std::string& file_name) : file_{file_name, std::ios::in | std::ios::binary} {
    file_.unsetf(std::ios::skipws);
  }

  /// destructor, close the input file stream
  ~Reader() = default;

  /**
   * Go ("seek") a specific position in the file.
   */
  void seek(int off, std::ios_base::seekdir dir = std::ios::beg) {
    file_.seekg(off, dir);
  }

  /**
   * Seek by number of words
   */
  template<typename WordType>
  void seek(int off, std::ios_base::seekdir dir = std::ios::beg) {
    seek(off*sizeof(WordType), dir);
  }

  /**
   * Tell us where the reader is
   */
  int tell() { return file_.tellg(); }

  /**
   * Tell by number of words
   */
  template<typename WordType>
  int tell() { return tell()/sizeof(WordType); }

  /**
   * Read the next 'count' words into the input handle.
   */
  template <typename WordType>
  Reader& read(WordType* w, std::size_t count) {
    static_assert(std::is_integral<WordType>::value, "Integral type required for Reader::read.");
    file_.read(reinterpret_cast<char*>(w), sizeof(WordType)*count);
    file_.seekg(sizeof(WordType)*count, std::ios::cur);
    return *this;
  }

 private:
  /// file stream we are reading from
  std::ifstream file_;
};  // RawDataFile

template <typename WordType>
Reader& operator>>(Reader& r, WordType w) {
  return r.read(&w, 1);
}

}  // namespace rawdatafile
}  // namespace packing

#endif  // PACKING_RAWDATAFILE_READER_H_
