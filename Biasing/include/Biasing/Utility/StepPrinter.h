#ifndef BIASING_UTILITY_STEPPRINTER_H
#define BIASING_UTILITY_STEPPRINTER_H

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

namespace biasing {
namespace utility {

/**
 * User stepping action used to print the details of a step.
 */
class StepPrinter : public simcore::UserAction {
 public:
  /**
   * Constructor.
   *
   * @param[in] name the name of the instance of this UserAction.
   * @param[in] parameters the parameters used to configure this
   *      UserAction.
   */
  StepPrinter(const std::string& name, framework::config::Parameters& parameters);

  /// Destructor
  ~StepPrinter();

  /**
   * Stepping action called when a step is taken during tracking of
   * a particle.
   *
   * @param[in] step Geant4 step
   */
  void stepping(const G4Step* step) final override;

  /// Retrieve the type of actions this class defines
  std::vector<simcore::TYPE> getTypes() final override {
    return {simcore::TYPE::STEPPING};
  }

 private:
  /// The track ID to filter on
  int trackID_{-9999};

};  // StepPrinter

}  // namespace utility
}  // namespace biasing

#endif  // BIASING_UTILITY_STEPPRINTER_H
