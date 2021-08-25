#ifndef TOOLS_CRC_H_
#define TOOLS_CRC_H_

#include <boost/crc.hpp>
#include <string_view>

namespace tools {

/**
 * @struct CRC
 *
 * The HGC ROC and FPGA use a CRC checksum to double check that the
 * data transfer has been done correctly. Boost has a CRC checksum
 * library that we can use to do this checking here as well.
 *
 * Idea for this helper struct was found on StackOverflow
 * https://stackoverflow.com/a/63237679
 *
 * This can now be used to 'write' a checksum pretty directly.
 *
 *  CRC my_sum;
 *  my_sum(42);
 *  my_sum("Hello World");
 *  my_sum(0.511);
 *  auto the_result{my_sum.get()};
 */
struct CRC {
  /// the checksum calculator object
  boost::crc_32_type crc;
  /// add a string of data to the checksum
  void operator()(std::string_view s) {
    crc.process_bytes(s.data(), s.size());
  }
  /// add any integral type to the checksum
  template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
  void operator()(T const& i) {
    static_assert(std::is_trivial_v<T>);
    static_assert(not std::is_class_v<T>);
    crc.process_bytes(&i, sizeof(i));
  }

  /// get the checksum from the calculator
  auto get() { return crc.checksum(); }
};

}  // namespace tools

#endif  // TOOLS_CRC_H_
