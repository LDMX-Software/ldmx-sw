#include "Ecal/Event/EcalMipResult.h"

ClassImp(ldmx::EcalMipResult);

namespace ldmx {

EcalMipResult::~EcalMipResult() { Clear(); }

std::ostream& operator<<(std::ostream& o, const EcalMipResult& c) {
  return o << "\t n_straight_tracks : " << c.n_straight_tracks_ << "\n"
           << "\t n_linreg_tracks : " << c.n_linreg_tracks_ << "\n"
           << "\t first_near_ph_layer : " << c.first_near_ph_layer_ << "\n"
           << "\t n_near_ph_hits : " << c.n_near_ph_hits_ << "\n"
           << "\t photon_territory_hits : " << c.photon_territory_hits_;
}

void EcalMipResult::Clear() {
  n_straight_tracks_ = 0;
  n_linreg_tracks_ = 0;
  first_near_ph_layer_ = 0;
  n_near_ph_hits_ = 0;
  photon_territory_hits_ = 0;
}

void EcalMipResult::setVariables(int n_straight_tracks, int n_linreg_tracks,
                                 int first_near_ph_layer, int n_near_ph_hits,
                                 int photon_territory_hits) {
  n_straight_tracks_ = n_straight_tracks;
  n_linreg_tracks_ = n_linreg_tracks;
  first_near_ph_layer_ = first_near_ph_layer;
  n_near_ph_hits_ = n_near_ph_hits;
  photon_territory_hits_ = photon_territory_hits;
}

}  // namespace ldmx