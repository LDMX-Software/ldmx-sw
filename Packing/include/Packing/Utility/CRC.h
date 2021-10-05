#ifndef PACKING_UTILITY_CRC_H_
#define PACKING_UTILITY_CRC_H_

#include <boost/crc.hpp>

namespace packing {
namespace utility {

/**
 * @class CRC
 *
 * The HGC ROC and FPGA use a CRC checksum to double check that the
 * data transfer has been done correctly. Boost has a CRC checksum
 * library that we can use to do this checking here as well.
 *
 * Idea for this helper struct was found on StackOverflow
 * https://stackoverflow.com/a/63237679
 * I've actually simplified it by limiting its use-case to our
 * type of data which is integral types, classes with the 'add'
 * method defined, and vectors of those types.
 *
 * This is a very light class and is meant to be used as a calculator.
 * It involves some fancy-shamncy template specialization using
 * std::enable_if in order for the streaming operator to function.
 *
 * ## Example
 *
 *  class MyObject {
 *   public:
 *    CRC& add(CRC& c) {
 *      c << my_member_int_;
 *    }
 *   private:
 *    uint64_t my_member_int_;
 *  } my_object; 
 *
 *  CRC c;
 *  c << 0xffff << std::vector<uint16_t>({0xffff, 0x00ff}) << my_object;
 *  uint32_t the_sum = c.get();
 *
 * ## Limitations
 *
 * This design limits the inputs to the calculator to two main categories.
 *  1. Integral types (e.g. bool, int, unsigned int, long, char, ...)
 *  2. Classes with the 'CRC& add(CRC&)' method defined
 *  3. A std::vector of objects in (1) or (2)
 * This means you will get a compiler error if you attempt to stream an
 * object not fitting into one of these categories.
 */
class CRC {
 public:
  /**
   * Stream an integral type into the calculator.
   *
   * This is only enabled for integral types i.e.
   * it looks like this definition doesn't exist for
   * types that are non-integral. Look at the C++ reference
   * for std::is_integral to see a full list of integral types.
   *
   * @param w integral-type word to insert into calculator
   * @return CRC modified calculator
   */
  template <typename WordType, std::enable_if_t<std::is_integral<WordType>::value,bool> = true>
  CRC& operator<<(const WordType& w) { 
    crc.process_bytes(&w, sizeof(WordType));
    return *this;
  }

  /**
   * Stream an instance of a class into the calculator
   *
   * Here, we assume that the class we are streaming
   * into the calculator has a specific public method defined.
   *
   *  CRC& add(CRC& c)
   *
   * and the user can insert the subcomponents to the calculator
   * within this method.
   *
   * @param o instance of an arbitrary class
   * @return CRC modified calculator
   */
  template <typename ObjectType, std::enable_if_t<std::is_class<ObjectType>::value,bool> = true>
  CRC& operator<<(const ObjectType& o) {
    return o.add(*this);
  }
  
  /**
   * Stream a vector of objects into the calculator
   *
   * When the compiler deduces that the input to the stream is a vector,
   * we simply call the stream operator on all members of the vector in
   * sequence.
   *
   * @param[in] vec vector of objects to insert into calculator
   * @return CRC modified calculator
   */
  template <typename ContentType>
  CRC& operator<<(const std::vector<ContentType>& vec) { 
    for (auto const& w : vec) *this << w;
    return *this;
  }

  /**
   * Get the calculate checksum from the calculator
   * @return uint32_t checksum
   */
  uint32_t get() { return crc.checksum(); }
 private:
  /// the object from Boost doing the summing
  boost::crc_32_type crc;
};  // CRC

}  // namespace utility
}  // namespace packing

#endif  // PACKING_UTILITY_CRC_H_
