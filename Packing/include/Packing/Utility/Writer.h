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
 * Writing a raw data file.
 *
 * We wrap a basic std::ifstream in order to make the writing
 * of specific-width words easier for ourselves.
 */
class Writer {
 public:
  /**
   * default constructor
   *
   * make sure we don't skip "whitespace"
   */
  Writer() {
    file_.unsetf(std::ios::skipws);
  }

  /**
   * Open a file with this writer
   *
   * We open the file stream in output, binary mode.
   *
   * @param[in] file_name name of file to open
   */
  void open(const std::string& file_name) {
    file_.open(file_name, std::ios::out | std::ios::binary);
  }

  /**
   * Open the input file name upon construction of this writer.
   *
   * @param[in] file_name name of file to open
   */
  Writer(const std::string& file_name) : Writer() {
    this->open(file_name);
  }

  /// destructor, close the input file stream
  ~Writer() = default;

  /**
   * Write a certain number of words from the input array to the output file stream.
   *
   * This method is only enabled for integral types so we can safely reinterpret
   * the underlying data as an array of characters.
   *
   * @tparam[in] WordType integral-type to write out
   * @param[in] w pointer to array of words to write
   * @param[in] num number of words in array
   * @return *this
   */
  template <typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  Writer& write(const WordType* w, std::size_t num) {
    file_.write(reinterpret_cast<const char*>(w), sizeof(WordType)*num);
    return *this;
  }

  /**
   * Write a single integral-type word to the output file stream.
   *
   * @see write
   *
   * @tparam[in] WordType integral-type to write out
   * @param[in] w single word to write out
   * @return *this
   */
  template <typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  Writer& operator<<(const WordType& w) {
    return write(&w, 1);
  }

  /**
   * Write out a vector of integral-type objects.
   *
   * This is a short-cut in order to help speed up writing.
   *
   * @tparam[in] WordType integral-type to write out
   * @param[in] vec vector of integral words to write out
   * @return *this
   */
  template <typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  Writer& operator<<(const std::vector<WordType>& vec) {
    return write(vec.data(), vec.size());
  }

  /**
   * Write out a class object.
   *
   * We assume that the input class object has a specific method defined.
   *
   *  Writer& write(Writer&) const;
   *
   * With this method defined, then the class can be streamed out through
   * this object.
   *
   * @tparam[in] ObjectType class-type to write out
   * @param[in] o object to write out
   * @return *this
   */
  template <typename ObjectType, std::enable_if_t<std::is_class<ObjectType>::value,bool> = true>
  Writer& operator<<(const ObjectType& o) {
    return o.write(*this);
  }

  /**
   * Write out a vector fo class objects
   *
   * We make the same assumptions as when streaming out a single class object.
   * We leave early if any write action changes the file to a fail state.
   *
   * @tparam[in] ObjectType class-type inside vector to write out
   * @param[in] vec vector of objects to write out
   * @return *this
   */
  template <typename ObjectType, std::enable_if_t<std::is_class<ObjectType>::value,bool> = true>
  Writer& operator<<(const std::vector<ObjectType>& vec) {
    for (auto const& o : vec) if (!o.write(*this)) return *this;
    return *this;
  }

  /**
   * Check if writer is in a fail state
   *
   * Uses std::ofstream::fail to check on stream.
   *
   * @return true if stream is in fail state
   */
  bool operator!() const {
    return file_.fail();
  }

  /**
   * Check if writer is in a good/bad state
   *
   * Uses std::ofstream::fail to check on stream.
   *
   * @return true if stream is in good state
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
