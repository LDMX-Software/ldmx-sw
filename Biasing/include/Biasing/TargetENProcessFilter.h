/**
 * @file TargetENProcessFilter.h
 * @brief Class defining a UserActionPlugin that biases Geant4 to only process
 *        events which involve an electronuclear reaction in the target
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef BIASING_TARGETENPROCESSFILTER_H
#define BIASING_TARGETENPROCESSFILTER_H

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

// Forward Declarations
class G4Step;
class G4Event;

namespace ldmx {

class TargetENProcessFilter : public UserAction {
 public:
  /**
   * Class constructor.
   */
  TargetENProcessFilter(const std::string& name, Parameters& parameters);

  /// Destructor
  ~TargetENProcessFilter();

  /**
   * Implementmthe stepping action which performs the target volume biasing.
   * @param step The Geant4 step.
   */
  void stepping(const G4Step* step) final override;

  /**
   * End of event action.
   */
  void EndOfEventAction(const G4Event*) final override;

  /// Retrieve the type of actions this class defines
  std::vector<TYPE> getTypes() final override {
    return {TYPE::EVENT, TYPE::STEPPING};
  }

 private:
  /** The volume name of the LDMX target. */
  std::string volumeName_{"target_PV"};

  /** Flag indicating if the reaction of intereset occurred. */
  bool reactionOccurred_{false};

  /** Energy that the recoil electron must not surpass. */
  double recoilEnergyThreshold_{1500};

  /// Process to filter on
  std::string process_{"electronNuclear"};

};  // TargetENProcessFilter

}  // namespace ldmx

#endif  // BIASING_TARGETPROCESSFILTER_H
