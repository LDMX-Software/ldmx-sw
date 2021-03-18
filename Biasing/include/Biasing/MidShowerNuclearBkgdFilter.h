#ifndef BIASING_MIDSHOWERBKGDFILTER_H_
#define BIASING_MIDSHOWERBKGDFILTER_H_

/*~~~~~~~~~~~~*/
/*   SimCore  */
/*~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/// Forward declaration of virtual process class
class G4VProcess;

namespace biasing {

/**
 * @class MidShowerNuclearBkgdFilter
 *
 * The basic premis of this filter is to add up all of the
 * energy "lost" to the configured process. Whenever a particle
 * makes a step and has secondaries within that step going to
 * the input process, we add its change in energy to the running
 * total for the event. When the PartialEnergySorter has run out
 * of "high" energy particles to process (when NewStage is called)
 * we check if the running total is high enough to keep the event.
 *
 * @see PartialEnergySorter
 * Here we assume that the partial energy sorter is being run in sequence with
 * this filter.
 */
class MidShowerNuclearBkgdFilter : public simcore::UserAction {
 public:
  /**
   * Class constructor.
   *
   * Retrieve the necessary configuration parameters
   */
  MidShowerNuclearBkgdFilter(const std::string& name,
                             framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  ~MidShowerNuclearBkgdFilter() {}

  /**
   * Get the types of actions this class can do
   *
   * @return list of action types this class does
   */
  std::vector<simcore::TYPE> getTypes() final override {
    return {simcore::TYPE::STACKING, simcore::TYPE::STEPPING,
            simcore::TYPE::EVENT};
  }

  /**
   * Reset the total energy going to the configured process.
   *
   * @param[in] event not used
   */
  void BeginOfEventAction(const G4Event* event) final override;

  /**
   * We follow the simulation along each step and check
   * if any secondaries of the input process were created.
   *
   * If they were created, we add the change in energy
   * to the total energy "lost" to the input process.
   *
   * @note Only includes the steps that are within the
   * 'CalorimeterRegion' in the search for interesting
   * products.
   *
   * @see isOutsideCalorimeterRegion
   * @see anyCreatedViaProcess
   *
   * @param[in] step current G4Step
   */
  void stepping(const G4Step* step) final override;

  /**
   * When using the PartialEnergySorter,
   * the *first* time that a new stage begins is when
   * all particles are now below the threshold.
   *
   * We take this opportunity to make sure that
   * enough energy has gone to the products of
   * the input process.
   *
   * @see PartialEnergySort::NewStage
   * @see AbortEvent
   */
  void NewStage() final override;

 private:
  /**
   * Checks if the passed step is outside of the CalorimeterRegion.
   *
   * @param[in] step const G4Step to check
   * @returns true if passed step is outside of the CalorimeterRegion.
   */
  bool isOutsideCalorimeterRegion(const G4Step* step) const;

  /**
   * Checks if the passed process is any of the nuclear interactions
   *
   * @note A 'nullptr' process is not a nuclear process.
   *
   * @param[in] proc G4VProcess* to check
   * @return true if process is one of the nuclear interactions
   */
  bool isNuclearProcess(const G4VProcess* proc) const;

  /**
   * Helper to save the passed track
   *
   * @note Assumes user track information has already been created
   * for the input track.
   *
   * @param[in] track G4Track to persist into output
   */
  void save(const G4Track* track) const;

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
   * Minimum energy [MeV] that the process products need to have
   * to keep the event.
   *
   * Also used by PartialEnergySorter to determine
   * which tracks should be processed first.
   *
   * Parameter Name: 'threshold'
   */
  double threshold_;

  /**
   * Processes to look for
   */
  std::vector<std::string> nuclear_processes_;

  /**
   * Total energy gone to the process in the current event
   *
   * Reset to 0. in BeginOfEventAction
   */
  double total_process_energy_{0.};

};  // MidShowerNuclearBkgdFilter
}  // namespace biasing

#endif  // BIASING_MIDSHOWERBKGDFILTER_H__
