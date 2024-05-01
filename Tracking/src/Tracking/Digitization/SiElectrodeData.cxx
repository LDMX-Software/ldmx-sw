#include "Tracking/Digitization/SiElectrodeData.h"

namespace tracking {
namespace digitization {

//TODO Change to operator overloading for cleaner code
SiElectrodeData SiElectrodeData::add(const SiElectrodeData& electrode_data) {
  add(electrode_data.getCharge(), electrode_data.getSimulatedHits());
  return *this;
}

SiElectrodeData SiElectrodeData::add(int charge,
                                     std::set<ldmx::SimTrackerHit> simulated_hits) {
  this->addCharge(charge);
  for (auto hit : simulated_hits) {
    this->addSimulatedHit(hit);
  }
  return *this;
}

SiElectrodeData SiElectrodeData::addCharge(int charge) {
  charge_+= charge;
  return *this;
}

SiElectrodeData SiElectrodeData::addSimulatedHit(const ldmx::SimTrackerHit hit) {
  sim_hits_.insert(hit);
  return *this;
}

}//digitization
}//tracking
