#ifndef REFORMAT_UTILITY_MASK_H_
#define REFORMAT_UTILITY_MASK_H_

namespace reformat {
namespace utility {

/**
 * mask_backend
 *
 * The backend struct for the mask generation scheme.
 * Since we are using a template parameter rather than
 * a function argument, these masks are generated at
 * compile-time and so are equivalent to defining a set
 * of static const varaibles.
 */
template <short N>
struct mask_backend {
  /// value of mask
  static const uint64_t value = (1ul << N) - 1ul;
};

/**
 * Generate bit masks at compile time.
 *
 * The template input defines how many of
 * the lowest bits will be masked for.
 *
 * Use like:
 *
 *  i & mask<N>
 *
 * To mask for the lowest N bits in i.
 *
 * Maximum mask is 63 bits because we are using
 * a 64-bit wide integer and so we can't do
 *  1 << 64
 * without an error being thrown during compile-time.
 *
 * @tparam N number of lowest-order bits to mask
 */
template <short N>
inline constexpr uint64_t mask = mask_backend<N>::value;

}  // namespace utility
}  // namespace reformat

#endif  // REFORMAT_UTILITY_MASK_H_
