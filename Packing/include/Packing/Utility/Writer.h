#ifndef PACKING_UTILITY_WRITER_H_
#define PACKING_UTILITY_WRITER_H_

#include <iostream> //debuggin
#include <fstream>
#include <string>
#include <type_traits>

namespace packing {
namespace utility {

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
    file_.write(reinterpret_cast<char*>(w), sizeof(WordType)*num);
    return *this;
  }

  template <typename WordType>
  Writer& operator<<(WordType& w) {
    return write(&w, 1);
  }
  
  /**
   * Write the vector.
   * We don't need 'num' because vectors container their length.
   */
  template <typename WordType>
  Writer& write(std::vector<WordType> vec) {
    return this->write(vec.data(), vec.size());
  }
  
  template <typename WordType>
  Writer& operator<<(std::vector<WordType>& vec) {
    return write(vec);
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

}  // namespace utility
}  // namespace packing

#endif  // PACKING_UTILITY_WRITER_H_
