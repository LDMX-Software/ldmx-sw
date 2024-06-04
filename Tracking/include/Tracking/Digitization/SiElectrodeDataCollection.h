#pragma once

#include <map>

#include "Tracking/Digitization/SiElectrodeData.h"

namespace tracking {
namespace digitization {

class SiElectrodeDataCollection {
 public:
  SiElectrodeDataCollection(){};
  ~SiElectrodeDataCollection() { collection_.clear(); };

  SiElectrodeDataCollection(const SiElectrodeDataCollection& electrode_data);

  // Create from a map of electrode charges for a single SimTrackerHit
  // map should always be sorted ascending
  SiElectrodeDataCollection(const std::map<int, int>& electrode_charge,
                            ldmx::SimTrackerHit hit);

  void add(const std::map<int, SiElectrodeData>& electrode_data_collection);

  void add(int cellid, SiElectrodeData electrode_data);

  std::map<int, int> getChargeMap() const;

  std::map<int, SiElectrodeData> getCollection() const { return collection_; };

  void clear() { collection_.clear(); };

 private:
  std::map<int, SiElectrodeData> collection_;
};

}  // namespace digitization
}  // namespace tracking
