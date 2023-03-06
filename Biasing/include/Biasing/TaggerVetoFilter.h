#ifndef BIASING_TAGGERVETOFILTER_H
#define BIASING_TAGGERVETOFILTER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <string>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"

// Forward declarations
class G4Step;

namespace biasing {

/**
 * User stepping action used to filter events that see the incident electron
 * fall below an energy threshold before reaching the target.
 *
 */
class TaggerVetoFilter : public simcore::UserAction {
public:
  /**
   * Constructor.
   *
   * @param[in] name the name of the instance of this UserAction.
   * @param[in] parameters the parameters used to configure this
   *      UserAction.
   */
  TaggerVetoFilter(const std::string &name,
                   framework::config::Parameters &parameters);

  /// Destructor
  ~TaggerVetoFilter();

  /**
   *
   * Action called at the start of an event to reset
   *
   */
  void BeginOfEventAction(const G4Event *) final override;
  /**
   *
   * Action called at the end of an event to veto events where the primary
   * particle never entered the tagger region.
   *
   */

  void EndOfEventAction(const G4Event *) final override;
  /**
   * Stepping action called when a step is taken during tracking of
   * a particle.
   *
   * @param[in] step Geant4 step
   */
  void stepping(const G4Step *step) final override;

  /// Retrieve the type of actions this class defines
  std::vector<simcore::TYPE> getTypes() final override {
    return {simcore::TYPE::STEPPING, simcore::TYPE::EVENT};
  }

private:
  /**
   * Did the primary particle enter the tagger region? Reset at the start of
   * each event
   *
   */
  bool primary_entered_tagger_region_{false};

  /// Energy below which an incident electron should be vetoed.
  double threshold_{0};

  // Should the EndOfEventAction reject events where the primary particle never
  // entered the tagger region?
  bool reject_primaries_missing_tagger_{true};

}; // TaggerVetoFilter

} // namespace biasing

#endif // BIASING_TAGGERVETOFILTER_H
