#ifndef PACKING_BUFFER_H_
#define PACKING_BUFFER_H_

#include <vector>
#include <stdexcept>

namespace packing {
namespace utility {

/**
 * @class BufferReader
 * This class is a helper class for reading the buffer
 * stored in the raw data format.
 *
 * The raw data format specifies that the buffer for all
 * data is a specific type (vector of bytes),
 * but the users in different subsystems/translators may
 * want the buffer to be read in different size words
 * than single bytes. This class helps in that translation.
 *
 * A basic idea for using this reader is provided in the
 * Ecal/Hcal raw decoders.
 *
 *    BufferReader<uint32_t> r{buffer_};
 *    while (r >> head1 >> head2) {
 *      // do decoding, using
 *      r >> w;
 *      // to get next word in buffer
 *    }
 *
 * @tparam[in] WordType type of word user wants to read out from buffer
 */
template <typename WordType>
class BufferReader {
 public:
  /**
   * Initialize a reader by wrapping a buffer to read.
   */
  BufferReader(const std::vector<uint8_t>& b) : buffer_{b}, i_word_{0} {}

  /**
   * Return state of buffer.
   * false if buffer is done being read,
   * true otherwise.
   */
  operator bool() {
    return (i_word_ < buffer_.size());
  }

  /**
   * Streaming operator
   * We get the next word if we are still in the buffer.
   * We always return ourselves so statements like
   *
   *  if (reader >> word1 >> word2)
   *
   * can correctly fail on the first or second word.
   *
   * @see next for implementation of reading the next word
   * @param[in] w reference to word to put data into
   * @return reference to us
   */
  BufferReader& operator>>(WordType& w) {
    if (*this)
      w = next();
    return *this;
  }
 
 private:
  /**
   * Go to next word in buffer.
   *
   * @note We assume that we are always in the buffer.
   * @throws std::out_of_range if try to go past end of buffer
   * @return next word in buffer
   */
  WordType next() {
    WordType w{0};
    for (std::size_t i_byte{0}; i_byte < n_bytes_; i_byte++) {
      w |= (buffer_.at(i_word_+i_byte) << 8*i_byte);
    }
    i_word_ += n_bytes_;
    return w;
  }

 private:
  // number of bytes in the words we are reading
  static const std::size_t n_bytes_{sizeof(WordType)};
  // current buffer we are reading
  const std::vector<uint8_t>& buffer_;
  // current index in buffer we are reading
  std::size_t i_word_;
};  // BufferReader

}  // namespace utility
}  // namespace packing

#endif  // PACKING_BUFFERREADER_H_
