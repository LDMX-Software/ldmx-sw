#ifndef SIMCORE_SCORINGPLANESD_H
#define SIMCORE_SCORINGPLANESD_H

#include "DetDescr/DetectorID.h"

#include "SimCore/SensitiveDetector.h"
#include "SimCore/Event/SimTrackerHit.h"

namespace simcore {

/**
 * Class defining a basic sensitive detector for scoring planes.
 */
class ScoringPlaneSD : public SensitiveDetector {
 public:
  /**
   * Constructor
   *
   * @param name The name of the sensitive detector.
   * @param ci Conditions interface handle
   * @param params python configuration parameters
   */
  ScoringPlaneSD(const std::string& name,
                 simcore::ConditionsInterface& ci,
                 const framework::config::Parameters& params);

  /** Destructor */
  ~ScoringPlaneSD();

  /**
   * Check if the input logical volume is a scoring plane we should include.
   */
  bool isSensDet(G4LogicalVolume* volume) const final override {
    return volume->GetName().contains(match_substr_);
  }

  /**
   * This is Geant4's handle to tell us that a particle has stepped
   * through our sensitive detector and we should process its interaction with us.
   *
   * @param[in] step the step that happened within one of our logical volumes
   * @param[in] hist the touchable history of the step
   */
  virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* hist) final override;

  /**
   * We are given the event bus here and we must decide
   * now what to persist into the event.
   *
   * @param[in,out] event event bus to add thing(s) to
   */
  virtual void saveHits(framework::Event& event) final override;

 private:
  /// Substring to match to logical volumes
  std::string match_substr_;
  
  /// Name of output collection to add
  std::string collection_name_;

  /// The actual output collection
  std::vector<ldmx::SimTrackerHit> hits_;

};  // ScoringPlaneSD

}  // namespace simcore

#endif  // SIMCORE_SCORINGPLANESD_H
