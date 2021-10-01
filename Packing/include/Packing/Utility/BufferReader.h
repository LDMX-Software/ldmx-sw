#ifndef PACKING_BUFFER_H_
#define PACKING_BUFFER_H_

namespace packing {

/**
 * The type of buffer used uniformly for all packed data
 */
typedef std::vector<uint64_t> BufferType;

/**
 * @class BufferReader
 * This class is a helper class for reading the buffer
 * stored in the raw data format.
 *
 * The raw data format specifies that the buffer for all
 * data is a specific type (BufferType),
 * but the users in different subsystems/translators may
 * want the buffer to be read in different size words
 * than what the word size of BufferType. This class helps in that translation.
 * Note that this reader starts on the zero'th word, 
 * so your 'while' loops should be 'do..while'.
 * A basic template for using this reader is
 *
 *    BufferReader<uint32_t> r{buffer_};
 *    do {
 *      try {
 *        r.now(); //first word in bunch of words
 *        r.next(); // another word that you are assuming exists
 *        ...other decoding...
 *      } catch(std::out_of_range& oor) {
 *        ldmx_log(debug) << oor.what();
 *        EXCEPTION_RAISE("MisFormat",
 *          "We were expecting another word in the format "
 *          "but there wasn't one!");
 *      }
 *    } while(r.next(false)); // its ok if the next word doesn't exist
 *                            // because we might be done decoding
 *
 * @tparam[in] WordType type of word user wants to read out from buffer
 */
template <typename WordType>
class BufferReader {
 public:
  /**
   * Initialize a reader by wrapping a buffer to read.
   */
  BufferReader(const BufferType& b) : buffer_{b}, i_read_{0}, i_subword_{0} {
    static_assert(
        std::is_same_v<WordType,uint64_t> or
        std::is_same_v<WordType,uint32_t> or
        std::is_same_v<WordType,uint16_t> or
        std::is_same_v<WordType,uint8_t >,
        "BufferReader::WordType must be one of "
        "uint64_t, uint32_t, uint16_t, uint8_t");
  }

  /**
   * Get the current word in buffer.
   * @return WordType current word
   */
  const WordType& now() {
    return reinterpret_cast<const WordType*>(&buffer_.at(i_read_))[i_subword_];
  }

  /**
   * Go to next word in buffer.
   *
   * @throws std::out_of_range if should_exist is true and we reach
   * the end of the buffer
   * @param[in] should_exist set to false if we are okay with the next word
   * not existing
   * @return true if we have another word, false if we've reached the end
   */
  bool next(bool should_exist = true) {
    if (i_subword_+1 == max_subwords_) {
      // next buffer word
      i_subword_ = 0;
      i_read_++;
      if (i_read_ == buffer_.size()) {
        if (should_exist)
          throw std::out_of_range("next word should exist");
        else
          return false;
      }
    } else {
      // next subword
      i_subword_++;
    }
    return true;
  }

 private:
  // maximum number of sub-words in a single buffer word
  static const std::size_t max_subwords_{sizeof(BufferType::value_type)/sizeof(WordType)};
  // current buffer we are reading
  const BufferType& buffer_;
  // current index in buffer we are reading
  std::size_t i_read_;
  // current sub-word in word in buffer we are reading
  std::size_t i_subword_;
};  // BufferReader

}  // namespace packing

#endif  // PACKING_BUFFERREADER_H_
