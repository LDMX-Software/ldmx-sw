#ifndef BIASING_PRIMARYTOECALFILTER_H
#define BIASING_PRIMARYTOECALFILTER_H

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

/**
 * User stepping action used to filter events where the primary
 * particle falls below a threshold before reaching the CalorimeterRegion
 *
 * This is a simplistic filter designed similar to the TaggerVetoFilter.
 */
class PrimaryToEcalFilter : public simcore::UserAction {
 public:
  /**
   * Constructor.
   *
   * @param[in] name the name of the instance of this UserAction.
   * @param[in] parameters the parameters used to configure this
   *      UserAction.
   */
  PrimaryToEcalFilter(const std::string& name, framework::config::Parameters& parameters);

  /// Destructor
  ~PrimaryToEcalFilter() {}

  /**
   * Only process if the track is a primary (parentID == 0) and
   * if the event is not aborted and the particle is not
   * in the CalorimeterRegion.
   *
   * If the energy of the particle is below the input threshold,
   * then the event is aborted.
   *
   * @param[in] step Geant4 step
   */
  void stepping(const G4Step* step) final override;

  /// Retrieve the type of actions this class defines
  std::vector<simcore::TYPE> getTypes() final override { return {simcore::TYPE::STEPPING}; }

 private:
  /// Energy [MeV] below which a primary should be vetoed.
  double threshold_;

};  // PrimaryToEcalFilter

}  // namespace biasing

#endif  // BIASING_PRIMARYTOECALFILTER_H
