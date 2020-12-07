#ifndef BIASING_UTILITY_WEIGHTBYSTEP_H_
#define BIASING_UTILITY_WEIGHTBYSTEP_H_

/*~~~~~~~~~~~~*/
/*   SimCore  */
/*~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

namespace biasing {
namespace utility {

/**
 * @class WeightByStep
 *
 * The basic setup for event weighting in Geant4 is designed
 * for biasing specific particles to do specific processes.
 * Then (when that process occurs), the user can take the weight
 * from the track defined as the product of the weights of all the
 * previous steps. This simple setup is useful for most applications
 * because the user can simply retrieve the track weight after
 * applying biasing without any further development.
 *
 * For LDMX, this basic setup is not enough. Specifically, for
 * some mid-shower backgrounds, we require weights from steps
 * at different locations on the shower tree. Since we are looking
 * at multiple particles, just multiplying the track weights together
 * can easily obtain mutiple factors of the same step weight.
 *
 * In order to remedy this issue, this action calculates a total
 * event weight by multiplying the weight for each individual step
 * into the event weight. Most steps will have a weight of one, but
 * the steps that are following a biased particle will have interesting
 * weights that need to be included in the final product.
 */
class WeightByStep : public ldmx::UserAction {
 public:
  /**
   * Class constructor.
   *
   * No parameters are used in this action.
   */
  WeightByStep(const std::string& name, ldmx::Parameters& parameters);

  /**
   * Class destructor.
   */
  ~WeightByStep() {}

  /**
   * Get the types of actions this class can do
   *
   * @return list of action types this class does
   */
  std::vector<ldmx::TYPE> getTypes() final override { return {ldmx::TYPE::STEPPING}; }

  /**
   * We follow the simulation along each step and
   * mutliply the weight for this individual step
   * into the overall event weight.
   *
   * @note On the first step of the event, we
   * create the UserEventInformation object and attach
   * it to the event. This is where the event weight
   * is stored for later saving into the output.
   *
   * @see UserEventInformation::incWeight
   *
   * @param[in] step current G4Step
   */
  void stepping(const G4Step* step) final override;

};  // WeightByStep
}  // namespace utility
}  // namespace biasing

#endif  // BIASING_UTILITY_WEIGHTBYSTEP_H_
