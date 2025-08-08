#include "Tracking/Event/Measurement.h"

ClassImp(ldmx::Measurement);

namespace ldmx {
Measurement::Measurement(const ldmx::SimTrackerHit& hit, const float& sigma_u,
                         const float& sigma_v) {
  // Set the global positions
  meas_x_ = hit.getPosition()[2];
  meas_y_ = hit.getPosition()[0];
  meas_z_ = hit.getPosition()[1];

  // Set the energy deposited
  edep_ = hit.getEdep();

  // Set the measurement time
  meas_t_ = hit.getTime();

  // Set the measurement ID
  id_ = hit.getID();

  // Set the local covariances
  cov_uu_ = sigma_u * sigma_u;
  cov_vv_ = sigma_v * sigma_v;

  // Store the trackID
  addTrackId(hit.getTrackID());
}

std::ostream& operator<<(std::ostream& output, const Measurement& measurement) {
  return output << "[ Measurement ]:\n\tGlobal position (mm): [ "
                << measurement.meas_x_ << ", " << measurement.meas_y_ << ", "
                << measurement.meas_z_ << " ]\n\tLocal position (mm): [ "
                << measurement.meas_u_ << ", " << measurement.meas_v_
                << "]\n\tcov(U,U) " << measurement.cov_uu_ << " cov(V,V) "
                << measurement.cov_vv_ << "\n\tTime: " << measurement.meas_t_
                << " ns \n\tLayer ID: " << measurement.layer_id_
                << "\n\tLayer: " << measurement.layer_
                << "\n\tEnergy Deposition: " << measurement.edep_ << " MeV";

  return output;
}
}  // namespace ldmx
