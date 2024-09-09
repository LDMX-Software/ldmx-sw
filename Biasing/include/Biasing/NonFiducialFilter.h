/**
 * @file NonFiducialFilter.h
 * @class NonFiducialFilter
 * @brief Class defining a UserActionPlugin that allows a user to filter out
 *        non-fiducial events,  i.e.when  the electron is not contained
 *        in the ECAL and so can act like the signal
 * @author David Jiang, UCSB
 * @author Tamas Almos Vami, UCSB
 */

#ifndef BIASING_NONFIDUCIALFILTER_H
#define BIASING_NONFIDUCIALFILTER_H

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
#include "Framework/EventProcessor.h"

namespace biasing {

/**
 * User action that allows a user to filter out events that are non-fiducial,
 * i.e. the electron is not contained in the ECAL and so can act like the signal
 */
class NonFiducialFilter : public simcore::UserAction {
 public:
  /// Constructor
  NonFiducialFilter(const std::string& name,
                    framework::config::Parameters& parameters);

  /// Destructor
  virtual ~NonFiducialFilter() = default;

  /**
   * Implement the stepping action which performs the target volume biasing.
   * @param step The Geant4 step.
   */
  void stepping(const G4Step* step) override;

  /**
   * Method called at the end of every event.
   * @param event Geant4 event object.
   */
  void EndOfEventAction(const G4Event*) override;

  /// Retrieve the type of actions this class defines
  std::vector<simcore::TYPE> getTypes() override {
    return {simcore::TYPE::EVENT, simcore::TYPE::STACKING,
            simcore::TYPE::STEPPING};
  }

 private:
  /// Recoil electron threshold.
  double recoil_max_p_{1500};  // MeV
  /// If turned on, this aborts fiducial events.
  bool abort_fiducial_{true};
  /// Enable logging
  enableLogging("NonFiducialFilter")

};  // NonFiducialFilter
}  // namespace biasing

#endif  // BIASING_NONFIDUCIALFILTER_H
