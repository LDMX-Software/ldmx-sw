#include "Tracking/Digitization/SiElectrodeDataCollection.h"

namespace tracking {
namespace digitization {

SiElectrodeDataCollection::SiElectrodeDataCollection(
    const SiElectrodeDataCollection& electrode_data) {
  collection_ = electrode_data.getCollection();
}

SiElectrodeDataCollection::SiElectrodeDataCollection(
    const std::map<int, int>& electrode_charge, ldmx::SimTrackerHit hit) {
  for (auto pair : electrode_charge) {
    collection_[pair.first] = SiElectrodeData(pair.second, hit);
  }
}

std::map<int, int> SiElectrodeDataCollection::getChargeMap() const {
  std::map<int, int> charge_map;

  for (auto pair : collection_) {
    charge_map[pair.first] = pair.second.getCharge();
  }

  return charge_map;
}

void SiElectrodeDataCollection::add(
    const std::map<int, SiElectrodeData>& electrode_data_collection) {
  for (auto pair : electrode_data_collection) {
    // Check if the internal collection_ has a key
    if (collection_.count(pair.first))
      collection_[pair.first].add(pair.second);

    else
      collection_[pair.first] = pair.second;
  }
}

void SiElectrodeDataCollection::add(int cellid,
                                    SiElectrodeData electrode_data) {
  if (electrode_data.isValid())
    if (collection_.count(cellid))
      collection_[cellid].add(electrode_data);
    else
      collection_[cellid] = electrode_data;
}

}  // namespace digitization
}  // namespace tracking
