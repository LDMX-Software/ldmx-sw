/**
 * @file PackedIndex.h
 * @brief Class which represents a maximally-packed index of up to four fields
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef DETDESCR_PACKEDINDEX_H_
#define DETDESCR_PACKEDINDEX_H_

#include <stdint.h>

namespace ldmx {

/**
 * @class PackedIndex
 * A maximally-packed index of up to four different fields.
 *
 * We can pack an index with different fields by knowing each
 * field's modulus or the number of different values the field can have.
 *
 * For example, if a field has values 0, 1, 2, 3 then its
 * modulus is four.
 *
 * The maths underlying this packing also assumes that values start
 * with 0 and count up.
 *
 * @tparam[in] m0 modulus of field 0 
 * @tparam[in] m1 modulus of field 1, optional
 * @tparam[in] m2 modulus of field 2, optional
 */
template <unsigned int m0, unsigned int m1=0, unsigned int m2=0>
class PackedIndex {
 public:
  /**
   * Constructor from field values
   * Put our values into a single 32-bit integer.
   *
   * We are provided two to four values to pack.
   * 
   * @note We do not check if the values are below the maxima
   * defined by the template parameters.
   *
   * @param[in] v0 value of field 0
   * @param[in] v1 value of field 1
   * @param[in] v2 value of field 2
   * @param[in] v3 value of field 3
   */
  PackedIndex(unsigned int v0, unsigned int v1, unsigned int v2=0, unsigned int v3=0) {
    index_=v0+v1*m0+v2*m0*m1+v3*m0*m1*m2;
  }

  /// Constructor from index value
  PackedIndex(uint32_t value) : index_{value} {}

  /// Get the value of field 0
  unsigned int field0() const { return index_%m0; }
  /// Get the value of field 1
  unsigned int field1() const { return (index_/m0)%m1; }
  /// Get the value of field 2
  unsigned int field2() const { return (index_/(m0*m1))%m2; }
  /// Get the value of field 3
  unsigned int field3() const { return (index_/(m0*m1*m2)); }

  /// Get the fully packed index
  uint32_t value() const { return index_; }
  
 private:
  /// The 32-bit integer that is storing our fields
  uint32_t index_;

};  // PackedIndex
}

#endif // DETDESCR_PACKEDINDEX_H_
