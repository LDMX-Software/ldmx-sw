#ifndef PACKING_UTILITY_HEX_H_
#define PACKING_UTILITY_HEX_H_

#include <iomanip>

namespace packing {
namespace utility {

/**
 * @struct hex
 * A very simple wrapper enabling us to more easily
 * tell the output stream to style the input word
 * in hexidecimal format.
 *
 * @tparam[in] WordType type of word for styling
 */
template <typename WordType>
struct Hex {
  static const std::size_t WIDTH{8 * sizeof(WordType)};
  WordType& word_;
  Hex(WordType& w) : word_{w} {}
  friend inline std::ostream& operator<<(
      std::ostream& os, const packing::utility::Hex<WordType>& h) {
    os << "0x" << std::setfill('0') << std::setw(h.WIDTH) << std::hex << h.word_
       << std::dec;
    return os;
  }
};

}  // namespace utility
}  // namespace packing

#endif  // PACKING_UTILITY_HEX_H_
