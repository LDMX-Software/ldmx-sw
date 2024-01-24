#ifndef BIASING_ENDZFILTER_H
#define BIASING_ENDZFILTER_H

//----------------//
//   C++ StdLib   //
//----------------//
#include <algorithm>

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"

namespace biasing {

/**
 * User action that allows a user to filter out events that don't occur deep enough in the ecal
 */
class EndZFilter : public simcore::UserAction {
 public:
  /// Constructor
  EndZFilter(const std::string& name,
                   framework::config::Parameters& parameters);

  /// Destructor
  ~EndZFilter();

  /**
   * Implement the stepping action which performs the target volume biasing.
   * @param step The Geant4 step.
   */
  void stepping(const G4Step* step) final override;

  /**
   * Method called at the end of every event.
   *
   * @param event Geant4 event object.
   */
  void EndOfEventAction(const G4Event* event) final override;

  /// Retrieve the type of actions this class defines
  std::vector<simcore::TYPE> getTypes() final override {
    return {simcore::TYPE::EVENT, simcore::TYPE::STACKING,
            simcore::TYPE::STEPPING};
  }

 private:

  double endZ_{300};

};  // EndZFilter
}  // namespace biasing

#endif  // BIASING_ENDZFILTER_H
