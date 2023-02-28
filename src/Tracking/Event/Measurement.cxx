#include "Tracking/Event/Measurement.h"

ClassImp(ldmx::Measurement)

namespace ldmx {
Measurement::Measurement(const ldmx::SimTrackerHit& hit,
                         const float& sigma_u,
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
  var_r_ = sigma_u * sigma_u;
  var_z_ = sigma_v * sigma_v;
}
}
