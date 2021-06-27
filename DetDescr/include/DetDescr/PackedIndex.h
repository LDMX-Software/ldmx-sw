/**
 * @file PackedIndex.h
 * @brief Class which represents a maximally-packed index of up to four fields
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef DETDESCR_PACKEDINDEX_H_
#define DETDESCR_PACKEDINDED_H_

#include <stdint.h>

namespace ldmx {

template <unsigned int m0, unsigned int m1=0, unsigned int m2=0>
class PackedIndex {
 public:
  PackedIndex(unsigned int v0, unsigned int v1, unsigned int v2=0, unsigned int v3=0) {
    index_=v0+v1*m0+v2*m0*m1+v3*m0*m1*m2;
  }
  PackedIndex(uint32_t value) : index_{value} { }

  unsigned int field0() const { return index_%m0; }
  unsigned int field1() const { return (index_/m0)%m1; }
  unsigned int field2() const { return (index_/(m0*m1))%m2; }
  unsigned int field3() const { return (index_/(m0*m1*m2)); }

  uint32_t value() const { return index_; }
  //  uint32_t operator() const { return index_; }
  
 private:
  uint32_t index_;
  };
}

#endif // DETDESCR_PACKEDINDED_H_
