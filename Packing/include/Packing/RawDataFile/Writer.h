#ifndef PACKING_RAWDATAFILE_WRITER_H_
#define PACKING_RAWDATAFILE_WRITER_H_

#include <iostream> //debuggin
#include <fstream>
#include <string>
#include <type_traits>

namespace packing {
namespace rawdatafile {

/**
 * @class Writer
 * Reading a raw data file.
 *
 * We wrap a basic std::ifstream in order to make the retrieving
 * of specific-width words easier for ourselves. Each reader has
 * a defined 'WordType' which makes us able to read-out data files,
 * getting words one or more at a time.
 */
class Writer {
 public:
  /// default constructor, make sure we don't skip "whitespace"
  Writer() {
    file_.unsetf(std::ios::skipws);
  }

  /**
   * Open a file with this writer
   */
  void open(const std::string& file_name) {
    file_.open(file_name, std::ios::out | std::ios::binary);
  }

  /// open the input file stream and get general info
  Writer(const std::string& file_name) : Writer() {
    this->open(file_name);
  }

  /// destructor, close the input file stream
  ~Writer() = default;

  /**
   * Write the 'num' words stored in array 'w'
   */
  template <typename WordType>
  Writer& write(WordType* w, std::size_t num) {
    static_assert(std::is_integral<WordType>::value, "Integral type required for Writer::write.");
    std::cout << "writing " << sizeof(WordType)*num << " bytes...";
    if (file_.write(reinterpret_cast<char*>(w), sizeof(WordType)*num)) {
      std::cout << "success";
    } else {
      std::cout << "failure";
    }
    std::cout << std::endl;

    return *this;
  }

  /**
   * Write the vector.
   * We don't need 'num' because vectors container their length.
   */
  template <typename WordType>
  Writer& write(std::vector<WordType> vec) {
    return this->write(vec.data(), vec.size());
  }

  /**
   * Check if writer is in a fail state
   */
  bool operator!() const {
    return file_.fail();
  }

  /**
   * Check if writer is in a good/bad state
   */
  operator bool() const {
    return !file_.fail();
  }

 private:
  /// file stream we are writing to
  std::ofstream file_;
};  // RawDataFile

template <typename WordType>
Writer& operator<<(Writer& writer, WordType& w) {
  return writer.write(&w, 1);
}

template <typename WordType>
Writer& operator<<(Writer& writer, std::vector<WordType>& vec) {
  return writer.write(vec);
}

}  // namespace rawdatafile
}  // namespace packing

#endif  // PACKING_RAWDATAFILE_WRITER_H_
