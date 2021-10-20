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
 * of specific-width words easier for ourselves.
 */
class Reader {
 public:
  /**
   * default constructor
   *
   * We make sure that our input file stream will not skip white space.
   */
  Reader() {
    file_.unsetf(std::ios::skipws);
  }

  /**
   * Open a file with this reader
   *
   * We open the file as an input, binary file.
   *
   * @param[in] file_name full path to the file we are going to open
   */
  void open(const std::string& file_name) {
    file_.open(file_name, std::ios::in | std::ios::binary);
    file_.seekg(0,std::ios::end);
    file_size_ = file_.tellg();
    file_.seekg(0);
  }

  /**
   * Constructor that also opens the input file
   * @see open
   * @param[in] file_name full path to the file we are going to open
   */
  Reader(const std::string& file_name) : Reader() {
    this->open(file_name);
  }

  /// destructor, close the input file stream
  ~Reader() = default;

  /**
   * Go ("seek") a specific position in the file.
   *
   * This non-template version of seek uses the default
   * meaning of the "off" parameter in which it counts bytes.
   *
   * @param[in] off number of bytes to move relative to dir
   * @param[in] dir location flag for the file, default is beginning
   */
  void seek(int off, std::ios_base::seekdir dir = std::ios::beg) {
    file_.seekg(off, dir);
  }

  /**
   * Seek by number of words
   *
   * This template version of seek uses the input word type
   * to move around by the count of the input words rather than
   * the count of bytes.
   *
   * @tparam[in] WordType Integral-type to count by
   * @param[in] off number of words to move relative to dir
   * @param[in] dir location flag for the file, default is beginning
   */
  template<typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  void seek(int off, std::ios_base::seekdir dir = std::ios::beg) {
    seek(off*sizeof(WordType), dir);
  }

  /**
   * Tell us where the reader is
   *
   * @return int number of bytes relative to beginning of file
   */
  int tell() { return file_.tellg(); }

  /**
   * Tell by number of words
   *
   * @return int number of words relative to beginning of file
   */
  template<typename WordType>
  int tell() { return tell()/sizeof(WordType); }

  /**
   * Read the next 'count' words into the input handle.
   *
   * This implementation of read is only available to pointers to integral types.
   * We assume that whatever space pointed to by w already has the space reserved
   * necessary for the input words.
   *
   * @tparam[in] WordType integral-type word to read out
   * @param[in] w pointer to WordType array to write data to
   * @param[in] count number of words to read
   * @return (*this)
   */
  template <typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  Reader& read(WordType* w, std::size_t count) {
    file_.read(reinterpret_cast<char*>(w), sizeof(WordType)*count);
    return *this;
  }

  /**
   * Stream the next word into the input handle
   *
   * This implementation of the stream operator is only available to handles of integral types.
   * Helps for shorthand of only grabbing a single word from the reader.
   *
   * @see read
   *
   * @tparam[in] WordType integral-type word to read out
   * @param[in] w reference to word to read into
   * @return handle to modified reader
   */
  template <typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  Reader& operator>>(WordType& w) {
    return read(&w, 1);
  }

  /**
   * Stream into a class object
   *
   * We assume that the class we are streaming to has a specific method defined.
   *
   *  Reader& read(Reader&)
   *
   * This can be satisfied by the classes that we own and operate.
   *
   * @tparam[in] ObjectType class type to read
   * @param[in] o instance of object to read into
   * @return *this
   */
  template <typename ObjectType, std::enable_if_t<std::is_class<ObjectType>::value,bool> = true>
  Reader& operator>>(ObjectType& o) {
    return o.read(*this);
  }
  
  /**
   * Read the next 'count' objects into the input vector.
   *  
   * This is common enough, I wanted to specialize the read function.
   *
   * The std::vector::resize is helpful for avoiding additional 
   * copies while the vector is being expanded. After allocating the space
   * for each entry in the vector, we call the stream operator from
   * *this into each entry in order, leaving early if a failure occurs.
   *
   * @tparam[in] ContentType type of object inside the vector
   * @param[in] vec object vector to read into
   * @param[in] count number of objects to read
   * @return *this
   */
  template <typename ContentType>
  Reader& read(std::vector<ContentType>& vec, std::size_t count) {
    vec.resize(count);
    for (auto& w : vec) {
      if(!(*this >> w)) return *this;
    }
    return *this;
  }

  /**
   * Check if reader is in a fail state
   *
   * Following the C++ reference, we pass-along the check
   * on if our ifstream is in a fail state.
   *
   * @return bool true if ifstream is in fail state
   */
  bool operator!() const {
    return file_.fail();
  }

  /**
   * Check if reader is in good/bad state
   *
   * Following the C++ reference, we pass-along the check
   * on if our ifstream is in a fail state.
   *
   * Defining this operator allows us to do the following.
   *
   * Reader r('dummy.raw')
   * if (r) {
   *   std::cout << "dummy.raw was opened good" << std::endl;
   * }
   *
   * @return bool true if ifstream is in good state
   */
  operator bool() const {
    return !file_.fail();
  }

  /**
   * check if file is done
   *
   * Just calls the underlying ifstream eof.
   *
   * @return true if we have reached the end of file.
   */
  bool eof() {
    return file_.eof() or file_.tellg() == file_size_;
  }

 private:
  /// file stream we are reading from
  std::ifstream file_;
  /// file size in bytes
  std::size_t file_size_;
};  // RawDataFile

}  // namespace utility
}  // namespace packing

#endif  // PACKING_UTILITY_READER_H_
