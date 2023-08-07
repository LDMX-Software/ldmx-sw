#ifndef SIMCORE_TRACKERSD_H
#define SIMCORE_TRACKERSD_H

#include "DetDescr/TrackerID.h"
#include "SimCore/Event/SimTrackerHit.h"
#include "SimCore/SensitiveDetector.h"

namespace simcore {

/**
 * @class TrackerSD
 * @brief Basic sensitive detector for trackers
 */
class TrackerSD : public SensitiveDetector {
 public:
  /**
   * Class constructor.
   * @param[in] name The name of the sensitive detector.
   * @param[in] ci conditions interface handle
   * @param[in] p parameters to configure sensitive detector
   */
  TrackerSD(const std::string& name, simcore::ConditionsInterface& ci,
            const framework::config::Parameters& p);

  /// Destructor
  ~TrackerSD();

  /**
   * Should the input logical volume be attached to
   * this sensitive detector?
   *
   * @note This is dependent on the naming convention in the GDML!
   */
  virtual bool isSensDet(G4LogicalVolume* volume) const final override {
    return (volume->GetName().contains("Sensor") and
            volume->GetName().contains(subsystem_));
  }

  /**
   * Process a step by creating a hit.
   *
   * @param step The step information
   * @param history The readout history.
   */
  G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);

  /**
   * Add the hits to the event and then reset the container
   */
  virtual void saveHits(framework::Event& event) final override {
    event.add(collection_name_, hits_);
  }

  virtual void EndOfEvent() final override {
    hits_.clear();
  }

 private:
  /// The name of the subsystem we are apart of
  std::string subsystem_;

  /// The name of the output collection
  std::string collection_name_;

  /// The collection of hits
  std::vector<ldmx::SimTrackerHit> hits_;

  /// The detector ID
  ldmx::SubdetectorIDType subDetID_;

};  // TrackerID

}  // namespace simcore

#endif  // SIMCORE_TRACKERSD_H
