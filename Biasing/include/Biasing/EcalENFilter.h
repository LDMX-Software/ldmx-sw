#ifndef BIASING_ECALENFILTER_H
#define BIASING_ECALENFILTER_H

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/UserAction.h"

// Forward Declarations
class G4Step;
class G4Event;

namespace ldmx {

/**
 * Action to look for EN products of at least a minimum total energy
 * within the ECal.
 *
 * Designed very similar to EcalBremFilter, but focused on summing
 * the EN products energy and not looking for a single product
 * of a certain energy.
 */
class EcalENFilter : public UserAction {
 public:
  /**
   * Class constructor.
   */
  EcalENFilter(const std::string& name, Parameters& parameters);

  /// Destructor
  ~EcalENFilter() {}

  /**
   * Will only process if event is not aborted and track is primary
   * (TrackID==1).
   *
   * If we step below the energy threshold or if we are stepping outside
   * of the calorimeter region, we get the secondaries. We loop through
   * the secondaries and add up all the products that a produced via
   * 'electronNuclear' process. If that energy is below the energy
   * threshold, we abort the event.
   *
   * @param step The Geant4 step.
   */
  void stepping(const G4Step* step) final override;

  /// Retrieve the type of actions this class defines
  std::vector<TYPE> getTypes() final override { return {TYPE::STEPPING}; }

 private:
  /// Minimum energy [MeV] that all electro-nuclear products must have
  double min_total_en_energy_;

};  // EcalENFilter

}  // namespace ldmx

#endif  // BIASING_TARGETPROCESSFILTER_H
