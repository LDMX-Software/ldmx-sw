/**
 * @file GenieBeamElectronKiller.h
 * @brief Stepping action that kills the primary beam electron when it enters
 *        the target region, for use with the GENIE full-event generator.
 */

#ifndef SIMCORE_G4USER_GENIEBEAMELECTRONKILLER_H
#define SIMCORE_G4USER_GENIEBEAMELECTRONKILLER_H

#include "SimCore/G4User/UserAction.h"

namespace simcore {

/**
 * @class GenieBeamElectronKiller
 * @brief Kills the primary beam electron when it reaches the target region.
 *
 * When using the GENIE generator with an upstream beam electron, the electron
 * needs to be tracked through the tagger to produce realistic hits, but should
 * be killed at the target since the GENIE interaction products replace the
 * electron's interaction. This stepping action monitors the primary electron
 * and kills it upon entering the "target" region.
 */
class GenieBeamElectronKiller : public UserAction {
 public:
  GenieBeamElectronKiller(const std::string& name,
                          framework::config::Parameters& parameters);

  ~GenieBeamElectronKiller() override = default;

  void stepping(const G4Step* step) override;

  std::vector<TYPE> getTypes() override { return {TYPE::STEPPING}; }

 private:
  enableLogging("GenieBeamElectronKiller")
};

}  // namespace simcore

#endif  // SIMCORE_G4USER_GENIEBEAMELECTRONKILLER_H
