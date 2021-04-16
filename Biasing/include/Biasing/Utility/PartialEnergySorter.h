#ifndef BIASING_UTILITY_PARTIALENERGYSORTER_H
#define BIASING_UTILITY_PARTIALENERGYSORTER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <string>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

// Forward declarations
class G4Step;

namespace biasing {
namespace utility {

/**
 * @class PartialEnergySorter
 * A simulation action that makes sure that all particles above a certain
 * threshold are processed first. This is not intended to modify any physics
 * or perform any filters, only to improve efficiency by processing the
 * more important, higher energy particles first before applying filtering.
 *
 * This UserAction can run on its own or combined with
 * other actions doing the filtering.
 *
 * @note The threshold in this sorter is for _kinetic_ energy.
 * It was implemented in this way to avoid processing slower-moving
 * but heavier particles, but you should keep this in mind when setting
 * the threshold.
 *
 * ## Necessities for Filters Run in Sequence
 * - *If* your derived class re-defines ClassifyNewTrack, you should avoid
 *   changing the classification and return the current track classification.
 * - *If* your derived class re-defines stepping, you should avoid
 *   change the track status.
 */
class PartialEnergySorter : public simcore::UserAction {
 public:
  /**
   * Constructor.
   *
   * @param[in] name the name of the instance of this UserAction.
   * @param[in] parameters the parameters used to configure this UserAction.
   */
  PartialEnergySorter(const std::string& name, framework::config::Parameters& parameters);

  /// Destructor
  virtual ~PartialEnergySorter() {}

  /**
   * At the beginning of a new event,
   * we reset the counter for the number of particles
   * above the energy threshold. This reset is
   * mandatory because sometimes previous events
   * will be aborted before all of the particles
   * above threshold are processed.
   *
   * @param[in] event unused
   */
  void BeginOfEventAction(const G4Event* event) final override;

  /**
   * Classify a "new" track
   *
   * This is called when a new track is created *or*
   * when a current track is suspended. We can use
   * this functionality to move any tracks below
   * the energy threshold to the waiting stack.
   *
   * All tracks with kinetic energy above the threshold are
   * put onto the urgent stack while any other particles
   * are put onto the waiting stack if there are particles
   * still above threshold.
   *
   * @param aTrack The Geant4 track.
   * @param currentTrackClass The current track classification.
   * @returns the updated classification
   */
  G4ClassificationOfNewTrack ClassifyNewTrack(
      const G4Track* aTrack,
      const G4ClassificationOfNewTrack& currentTrackClass) final override;

  /**
   * Checks if a particle steps from above the threshold to below it.
   * If the particle does step below the threshold it is suspended,
   * so that it can be re-classifed onto the waiting stack.
   *
   * Nothing happens if we are already below threshold
   * (i.e. below_threshold_ is true).
   *
   * @param[in] step Geant4 step
   */
  void stepping(const G4Step* step) final override;

  /// Retrieve the type of actions this class defines
  std::vector<simcore::TYPE> getTypes() {
    return {simcore::TYPE::STEPPING, simcore::TYPE::STACKING, simcore::TYPE::EVENT};
  }

  /**
   * Flag that we are now going below threshold.
   *
   * This function is called when the urgent stack is empty
   * and the waiting stack is transferred to the urgent stack.
   *
   * With all tracks below threshold being pushed to the waiting stack,
   * the first time this occurs during the event is when the rest
   * of the particles are below the threshold energy, so we reset the
   * counter on the number of particles above the threshold here.
   *
   * This function can be called several times in an event, but
   * the *first* time it is called in an event is when there is
   * no more particles with kinetic energy above the threshold
   * left to be processed.
   *
   * Sometimes (e.g. in the case when you are using this with the
   * custom dark brem process) there will still be particles
   * "left-over" that are above the threshold. These particles are
   * the electron that went dark brem and the A' which are above
   * the threshold but both don't interact with anything else.
   * Since they don't interact with anything else, they finish
   * tracking without "stepping" from above the threshold to
   * below it.
   */
  void NewStage() final override {
    /** debug printout
    std::cout << "[ PartialEnergySorter ] : "
        << "Starting new stage with "
        << num_particles_above_threshold_
        << " particles above threshold."
        << std::endl;
     */
    below_threshold_ = true;
  }

 private:
  /// Minimum Kinetic Energy [MeV] we want to simulate first
  double threshold_;

  /// Are we simulating below the threshold yet?
  bool below_threshold_;

};  // PartialEnergySorter

}  // namespace utility
}  // namespace biasing

#endif  // BIASING_UTILITY_PARTIALENERGYSORTER_H
