#include "Recon/Event/ECONDEventInfo.h"

ClassImp(ldmx::ECONDLinkInfo);
ClassImp(ldmx::ECONDSampleInfo);
ClassImp(ldmx::ECONDEventInfo);

namespace ldmx {

void ECONDEventInfo::clear() {
  corruption_ = 0;
  trailer_n_samples_ = -1;
  i_soi_ = 0;
  n_skipped_words_ = 0;
  digis_filled_ = false;
  samples_.clear();
}

std::ostream& operator<<(std::ostream& o, const ECONDLinkInfo& l) {
  return o << "ECONDLinkInfo { stat: " << l.stat_ << ", empty: " << l.is_empty_
           << ", cm0: " << l.adc_cm0_ << ", cm1: " << l.adc_cm1_
           << ", calib: " << l.calib_ << " }";
}

std::ostream& operator<<(std::ostream& o, const ECONDSampleInfo& s) {
  return o << "ECONDSampleInfo { econ: " << s.econ_id_
           << ", i_sample: " << s.i_sample_ << ", soi: " << s.is_soi_
           << ", length: " << s.payload_length_ << ", bx: " << s.bx_
           << ", l1a: " << s.l1a_ << ", orbit: " << s.orbit_
           << ", corruption: " << s.corruption_
           << ", n_links: " << s.links_.size() << " }";
}

std::ostream& operator<<(std::ostream& o, const ECONDEventInfo& e) {
  return o << "ECONDEventInfo { corruption: " << e.corruption_
           << ", n_samples: " << e.samples_.size()
           << ", trailer_n_samples: " << e.trailer_n_samples_
           << ", soi: " << e.i_soi_ << ", skipped: " << e.n_skipped_words_
           << ", digis_filled: " << e.digis_filled_ << " }";
}

}  // namespace ldmx
