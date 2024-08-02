#include "Tracking/Event/Measurement.h"

ClassImp(ldmx::Measurement)

    namespace ldmx {
  Measurement::Measurement(const ldmx::SimTrackerHit& hit, const float& sigma_u,
                           const float& sigma_v) {
    // Set the global positions
    x_ = hit.getPosition()[2];
    y_ = hit.getPosition()[0];
    z_ = hit.getPosition()[1];

    // Set the energy deposited
    edep_ = hit.getEdep();

    // Set the measurement time
    t_ = hit.getTime();

    // Set the measurement ID
    id_ = hit.getID();

    // Set the local covariances
    cov_uu_ = sigma_u * sigma_u;
    cov_vv_ = sigma_v * sigma_v;

    // Store the trackID
    addTrackId(hit.getTrackID());
  }

  std::ostream& operator<<(std::ostream& output,
                           const Measurement& measurement) {
    output << "[ Measurement ]:\n\tGlobal position (mm): [ " << measurement.x_
           << ", " << measurement.y_ << ", " << measurement.z_
           << " ]\n\tLocal position (mm): [ " << measurement.u_ << ", "
           << measurement.v_ << "]\n\tcov(U,U) " << measurement.cov_uu_
           << " cov(V,V) " << measurement.cov_vv_
           << "\n\tTime: " << measurement.t_
           << " ns \n\tLayer ID: " << measurement.layerid_
           << "\n\tLayer: " << measurement.layer_
           << "\n\tEnergy Deposition: " << measurement.edep_ << " MeV"
           << std::endl;

    return output;
  }
}
