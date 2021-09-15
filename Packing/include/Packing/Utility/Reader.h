#ifndef PACKING_UTILITY_READER_H_
#define PACKING_UTILITY_READER_H_

#include <iostream> //debuggin
#include <fstream>
#include <string>
#include <type_traits>

namespace packing {
namespace utility {

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
  /// default constructor
  Reader() {
    file_.unsetf(std::ios::skipws);
  }

  /**
   * Open a file with this reader
   */
  void open(const std::string& file_name) {
    file_.open(file_name, std::ios::in | std::ios::binary);
  }

  /// open the input file stream and get general info
  Reader(const std::string& file_name) : Reader() {
    this->open(file_name);
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
  template <typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  Reader& read(WordType* w, std::size_t count) {
    file_.read(reinterpret_cast<char*>(w), sizeof(WordType)*count);
    return *this;
  }

  template <typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  Reader& operator>>(WordType& w) {
    return read(&w, 1);
  }
  
  /**
   * Read the next 'count' words into the input vector of words
   *  
   * This is common enough, I wanted to specialize the read function.
   *
   * The std::vector::resize is helpful for avoiding additional 
   * copies while the vector is being expanded.
   */
  template <typename ContentType>
  Reader& read(std::vector<ContentType>& vec, std::size_t count) {
    vec.resize(count);
    for (auto& w : vec) {
      if(!(*this >> w)) return *this;
    }
    return *this;
  }

  template <typename ObjectType, std::enable_if_t<std::is_class<ObjectType>::value,bool> = true>
  Reader& operator>>(ObjectType& o) {
    o.read(*this);
    return *this;
  }

  /**
   * Check if reader is in a fail state
   */
  bool operator!() const {
    return file_.fail();
  }

  /**
   * Check if reader is in good/bad state
   */
  operator bool() const {
    return !file_.fail();
  }

  /// check if file is done
  bool eof() const {
    return file_.eof();
  }


 private:
  /// file stream we are reading from
  std::ifstream file_;
};  // RawDataFile

}  // namespace utility
}  // namespace packing

#endif  // PACKING_UTILITY_READER_H_
