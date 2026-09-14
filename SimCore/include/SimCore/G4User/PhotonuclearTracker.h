/**
 * @file PhotonuclearTracker.h
 * @brief UserAction for tracking detailed photonuclear interaction information
 * @author Tamas Almos Vami (UCSB)
 */

#ifndef SIMCORE_G4USER_PHOTONUCLEARTRACKER_H_
#define SIMCORE_G4USER_PHOTONUCLEARTRACKER_H_

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <map>
#include <memory>
#include <vector>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Step.hh"
#include "G4Track.hh"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/Event/PhotonuclearInteraction.h"
#include "SimCore/G4User/UserAction.h"

namespace simcore {

/**
 * @class PhotonuclearTracker
 * @brief Tracks detailed information about photonuclear interactions
 *
 * This UserAction captures comprehensive details about photonuclear (PN)
 * interactions during simulation, including:
 * - Incident photon kinematics
 * - Target nucleus information
 * - All immediate secondary particles from the cascade
 * - Mapping from cascade products to their final state descendants
 *
 * The tracker operates by:
 * 1. Detecting PN interactions in the stepping action (when secondaries are
 * created)
 * 2. Capturing all immediate secondaries and their G4 track IDs
 * 3. Tracking descendants through the event via ancestry propagation
 * 4. Building final state maps when tracks complete
 *
 * The collected interactions are saved to the event bus as a vector:
 *   "PhotonuclearInteractions"
 */
class PhotonuclearTracker : public UserAction {
 public:
  /**
   * Constructor
   *
   * @param name Name of this user action
   * @param parameters Configuration parameters
   */
  PhotonuclearTracker(const std::string& name,
                      framework::config::Parameters& parameters);

  /**
   * Destructor
   */
  virtual ~PhotonuclearTracker() = default;

  /**
   * Get the types of actions this class handles
   *
   * @return Vector of action types (EVENT, STEPPING, TRACKING)
   */
  std::vector<TYPE> getTypes() override {
    return {TYPE::EVENT, TYPE::STEPPING, TYPE::TRACKING};
  }

  /**
   * Called at the beginning of each event
   *
   * Clears the interaction collection and ancestry maps for the new event.
   *
   * @param event Geant4 event object
   */
  void beginOfEventAction(const G4Event* event) override;

  /**
   * Called at the end of each event
   *
   * Final cleanup and validation of collected data.
   *
   * @param event Geant4 event object
   */
  void endOfEventAction(const G4Event* event) override;

  /**
   * Called after each simulation step
   *
   * Detects PN interactions by examining secondaries created in the step.
   * When a PN interaction is found, captures all details and initializes
   * genealogy tracking.
   *
   * @param step Current Geant4 step
   */
  void stepping(const G4Step* step) override;

  /**
   * Called before tracking a new track
   *
   * Propagates ancestry information for descendants of PN secondaries.
   *
   * @param track Current Geant4 track
   */
  void preUserTrackingAction(const G4Track* track) override;

  /**
   * Called after tracking a track
   *
   * Records final state particles when tracks complete, building the
   * descendant map for PN interactions.
   *
   * @param track Current Geant4 track
   */
  void postUserTrackingAction(const G4Track* track) override;

  /**
   * Get the collection of PN interactions for this event
   *
   * @return Vector of PhotonuclearInteraction objects
   */
  std::vector<ldmx::PhotonuclearInteraction> getInteractions() const {
    return pn_interactions_;
  }

  /**
   * Get the current PhotonuclearTracker instance
   *
   * @return Pointer to the current instance, or nullptr if not created
   */
  static PhotonuclearTracker* get() { return instance; }

 private:
  /**
   * Extract target nucleus information from the current step
   *
   * @param step Current Geant4 step
   * @param Z Output: atomic number
   * @param A Output: mass number
   * @param material Output: material name
   */
  void extractTargetInfo(const G4Step* step, int& Z, int& A,
                         std::string& material);

  /**
   * Create a ParticleInfo struct from a G4Track
   *
   * @param track Geant4 track
   * @return ParticleInfo with kinematic data
   */
  ldmx::PhotonuclearInteraction::ParticleInfo createParticleInfo(
      const G4Track* track);

 private:
  /// Collection of PN interactions in this event
  std::vector<ldmx::PhotonuclearInteraction> pn_interactions_;

  /// Maps secondary track ID -> index in pn_interactions_
  /// Used to quickly find which PN interaction a secondary belongs to
  std::map<int, int> secondary_to_pn_index_;

  /// Maps descendant track ID -> immediate secondary track ID
  /// Propagates through the genealogy to trace back to the original PN
  /// secondary
  std::map<int, int> descendant_ancestry_;

  /// Static pointer to the current instance
  static PhotonuclearTracker* instance;

};  // PhotonuclearTracker

}  // namespace simcore

#endif  // SIMCORE_G4USER_PHOTONUCLEARTRACKER_H_
