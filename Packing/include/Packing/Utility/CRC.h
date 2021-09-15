#ifndef PACKING_UTILITY_CRC_H_
#define PACKING_UTILITY_CRC_H_

#include <boost/crc.hpp>

namespace packing {
namespace utility {

/**
 * @struct CRC
 *
 * The HGC ROC and FPGA use a CRC checksum to double check that the
 * data transfer has been done correctly. Boost has a CRC checksum
 * library that we can use to do this checking here as well.
 *
 * Idea for this helper struct was found on StackOverflow
 * https://stackoverflow.com/a/63237679
 * I've actually simplified it by limiting its use-case to our
 * type of data.
 */
struct CRC {
  // the object from Boost doing the summing
  boost::crc_32_type crc;
  // add a word to the sum
  template <typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  CRC& operator<<(const WordType& w) { 
    crc.process_bytes(&w, sizeof(WordType));
    return *this;
  }
  // add a word to the sum
  template <typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  CRC& operator<<(const std::vector<WordType>& vec) { 
    for (auto const& w : vec) (*this) << w;
  }
  // get the checksum
  auto get() { return crc.checksum(); }
};

}  // namespace utility
}  // namespace packing

#endif  // PACKING_UTILITY_CRC_H_
