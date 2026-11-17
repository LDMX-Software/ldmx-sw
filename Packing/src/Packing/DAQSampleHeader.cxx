#include "Packing/DAQSampleHeader.h"

#include "Packing/Utility/Mask.h"

using packing::utility::MASK;

namespace packing {

void DAQSampleHeader::from(uint32_t word) {
  version_ = ((word >> 28) & MASK<4>);
  econd_id_ = ((word >> 18) & MASK<10>);
  i_l1a_ = ((word >> 13) & MASK<5>);
  is_soi_ = (((word >> 12) & MASK<1>) == 1);
  econd_len_ = (word & MASK<12>);
}

uint32_t DAQSampleHeader::to() const {
  return (((version_ & MASK<4>) << 28) | ((econd_id_ & MASK<10>) << 18) |
          ((i_l1a_ & MASK<5>) << 13) | ((is_soi_ ? 1 : 0) << 12) |
          (econd_len_ & MASK<12>));
}

bool DAQSampleHeader::isValid(uint32_t word) {
  DAQSampleHeader h;
  h.from(word);
  return h.version_ == VERSION and not h.isEndingTrailer() and
         ((word >> 9) & MASK<3>) == 0;
}

bool DAQSampleHeader::isEndingTrailer() const {
  return ((i_l1a_ == 0x1f) and (econd_id_ == 0x3ff));
}

uint32_t DAQSampleHeader::trailerNumSamples(uint32_t word) {
  return (word & MASK<5>);
}

uint32_t DAQSampleHeader::endingTrailer(uint32_t n_samples) {
  DAQSampleHeader h{.version_ = VERSION,
                    .econd_id_ = 0x3ff,
                    .i_l1a_ = 0x1f,
                    .is_soi_ = false,
                    .econd_len_ = 0};
  return h.to() | (1u << 11) | (n_samples & MASK<5>);
}

std::ostream& operator<<(std::ostream& o, const DAQSampleHeader& h) {
  return (o << "DAQSampleHeader { " << "version: " << h.version_ << ", "
            << "econd_id: " << h.econd_id_ << ", " << "i_l1a: " << h.i_l1a_
            << ", " << "is_soi: " << h.is_soi_ << ", "
            << "econd_len: " << h.econd_len_ << " }");
}

}  // namespace packing
