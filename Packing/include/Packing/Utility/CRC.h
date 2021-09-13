#ifndef REFORMAT_UTILITY_CRC_H_
#define REFORMAT_UTILITY_CRC_H_

#include <boost/crc.hpp>

namespace reformat {
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
  void operator()(const uint32_t& w) { crc.process_bytes(&w, sizeof(w)); }
  // get the checksum
  auto get() { return crc.checksum(); }
};

}  // namespace utility
}  // namespace reformat

#endif  // REFORMAT_UTILITY_CRC_H_
