/**
 * @file EcalDarkBremFilter.h
 * @class EcalDarkBremFilter
 * @brief Class defining a simcore::UserActionPlugin that allows a user to
 * filter out events that don't result in a dark brem inside a given volume
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef BIASING_ECALDARKBREMFILTER_H_
#define BIASING_ECALDARKBREMFILTER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

//------------//
//   Geant4   //
//------------//
#include "G4RunManager.hh"

/*~~~~~~~~~~~~*/
/*   SimCore  */
/*~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

namespace biasing {

/**
 * @class EcalDarkBremFilter
 *
 * This class is meant to filter for events that produce a dark brem
 * occuring within the ECal and producing an A' of a minimum energy
 * threshold [MeV].
 *
 * @note The threshold that the A' needs to have should be equal to
 * or greater than the threshold of the PartialEnergySorter.
 *
 * @see PartialEnergySorter
 * Here we assume that the partial energy sorter is being run in sequence with
 * this filter. This filter makes sure that any events passing this filter have
 * an A' of the required input energy and generated inside of the required input
 * volume.
 */
class EcalDarkBremFilter : public simcore::UserAction {
 public:
  /**
   * Class constructor.
   *
   * Retrieve the necessary configuration parameters
   */
  EcalDarkBremFilter(const std::string& name,
                     framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  ~EcalDarkBremFilter() {}

  /**
   * Get the types of actions this class can do
   *
   * @return list of action types this class does
   */
  std::vector<simcore::TYPE> getTypes() final override {
    return {simcore::TYPE::STACKING, simcore::TYPE::EVENT,
            simcore::TYPE::TRACKING};
  }

  /**
   * Reset flag on if A' has been found
   *
   * @param event unused
   */
  void BeginOfEventAction(const G4Event* event) final override;

  /**
   * We return the classification of the track done by the PartialEnergySorter,
   * but we can check here if the A' has been created above the required
   * energy.
   *
   * Checks a new track for being an A' above threshold_
   *  if it is an A', sets the foundAp_ member
   *
   * @see PartialEnergySort::ClassifyNewTrack
   * @param aTrack The Geant4 track.
   * @param currentTrackClass The current track classification.
   * @returns current track classification
   */
  G4ClassificationOfNewTrack ClassifyNewTrack(
      const G4Track* aTrack,
      const G4ClassificationOfNewTrack& currentTrackClass) final override;

  /**
   * When using the PartialEnergySorter,
   * the *first* time that a new stage begins is when
   * all particles are now below the threshold.
   *
   * We take this opportunity to make sure that the A'
   * has been found.
   *
   * @see PartialEnergySort::NewStage
   */
  void NewStage() final override;

  /**
   * Make sure A' is saved.
   *
   * If passed track is A', set save status to true
   * Aborts event if A' does not originate in desired volume.
   * This is the last check that the event needs to pass
   * to be kept.
   *
   * @param track G4Track to check if it is an A'
   */
  void PostUserTrackingAction(const G4Track* track) final override;

 private:
  /**
   * Check if input volume is in the desired volume name
   *
   * @param pointer to track to check
   * @return true if originated in desired volume
   */
  bool inDesiredVolume(const G4Track*) const;

  /**
   * Helper to abort an event with a message
   *
   * Tells the RunManger to abort the current event
   * after displaying the input message.
   *
   * @param[in] reason reason for aborting the event
   */
  void AbortEvent(const std::string& reason) const;

 private:
  /**
   * Minimum energy [MeV] that the A' should have to keep the event.
   *
   * Also used by PartialEnergySorter to determine
   * which tracks should be processed first.
   *
   * Parameter Name: 'threshold'
   */
  double threshold_;

  /**
   * The volumes that the filter will be applied to.
   */
  std::vector<G4LogicalVolume*> volumes_;

  /**
   * Have we found the A' yet?
   *
   * Reset to false in BeginOfEventAction
   */
  bool foundAp_;

};  // EcalDarkBremFilter
}  // namespace biasing

#endif  // BIASING_ECALDARKBREMFILTER_H__
