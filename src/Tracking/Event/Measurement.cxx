#include "Tracking/Event/Measurement.h"

#include "Tracking/Sim/TrackingUtils.h"

ClassImp(ldmx::Measurement)

namespace ldmx {
Measurement::Measurement(const ldmx::SimTrackerHit& hit,
                         const float& sigma_u,
                         const float& sigma_v) {
  // Get the layer ID.
  layerid_ = tracking::sim::utils::getSensorID(hit);

  // Set the global positions
  x_ = hit.getPosition()[2];
  y_ = hit.getPosition()[0];
  z_ = hit.getPosition()[1];

  edep_ = hit.getEdep();
  t_ = hit.getTime();
  id_ = hit.getID();

  var_r_ = sigma_u * sigma_u;
  var_z_ = sigma_v * sigma_v;
}
}
