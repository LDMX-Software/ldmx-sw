#include "Packing/Utility/CRC.h"

#include <boost/crc.hpp>
#include <boost/endian/conversion.hpp>
#include <iomanip>
#include <iostream>
#include <vector>

namespace packing::utility {

uint32_t crc32(std::span<const uint32_t> data) {
  /**
   * We flip the endian-ness so that the bytes are processed
   * MSB to LSB after casting our array of words into an array of bytes.
   */
  std::vector<uint32_t> words{data.begin(), data.end()};
  std::transform(words.begin(), words.end(), words.begin(),
                 [](uint32_t w) { return boost::endian::endian_reverse(w); });
  auto input_ptr = reinterpret_cast<const unsigned char*>(words.data());
  /**
   * @note these CRC parameters are copied from the HGCROC but appear
   * to be correct for the ECOND implementation as well.
   */
  return boost::crc<32, 0x04c11db7, 0x0, 0x0, false, false>(input_ptr,
                                                            words.size() * 4);
}

uint8_t econd_crc8(uint64_t data) {
  /**
   * Reverse endian-ness so when we cast to an array of bytes,
   * we read the bytes from most-significant to least-significant.
   */
  uint64_t w = boost::endian::endian_reverse(data);
  auto input_ptr = reinterpret_cast<const unsigned char*>(&w);
  /**
   * These CRC parameters are what I can find online for 8bit Bluetooth CRC,
   * except the last two booleans RefIn and RefOut which are false instead
   * of the Bluetooth values of true (according to https://www.crccalc.com/).
   * I'm guessing we aren't having the CRC calculator reflecting because
   * we are already reflecting when constructing the byte stream?
   */

  /*
   * for future reference:
   * boost::crc<Bits, Poly, Init, XorOut, RefIn, RefOut>
   */
  return boost::crc<8, 0xa7, 0x00, 0x00, false, false>(input_ptr, 8);
}

}  // namespace pflib::utility
