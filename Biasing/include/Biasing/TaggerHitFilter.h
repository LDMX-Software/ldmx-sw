#pragma once

//~~ C++ StdLib ~~//
#include <set>
#include <string>

//~~ SimCore ~~//
#include "SimCore/UserAction.h"

//~~ Framework ~~//
#include "Framework/Configure/Parameters.h"

// Forward declarations
class G4Step;

namespace biasing {

/**
 * User stepping action used to filter events where the incident electron
 * fails to hit a minimum number of tracker sensors.
 */
class TaggerHitFilter : public simcore::UserAction {
 public:
  /**
   * Constructor.
   *
   * @param[in] name the name of the instance of this UserAction.
   * @param[in] parameters the parameters used to configure this
   *      UserAction.
   */
  TaggerHitFilter(const std::string& name,
                  framework::config::Parameters& parameters);

  /// Destructor
  ~TaggerHitFilter() = default;

  /**
   * Stepping action called when a step is taken during tracking of
   * a particle.
   *
   * @param[in] step Geant4 step
   */
  void stepping(const G4Step* step) final override;


  /**
   * Action called once tracking of all particles has concluded. This is being
   * used to clear the hit count set in preparation for the next event.
   */
  void EndOfEventAction(const G4Event* event) final override; 

  /// Retrieve the type of actions this class defines
  std::vector<simcore::TYPE> getTypes() final override {
    return {simcore::TYPE::STEPPING, simcore::TYPE::EVENT};
  }

 private:

  void checkAbortEvent(G4Track* track);  

  /// Set used to keep track which layers were hit by a particle.
  std::set<int> layer_count_;
  /// Total number of hits required to persist an event.
  int layers_hit_{8};


};  // TaggerHitFilter
}  // namespace biasing
