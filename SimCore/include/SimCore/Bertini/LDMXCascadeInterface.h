/**
 * @file LDMXCascadeInterface.h
 * @brief Bertini cascade interface with history capture
 */

#ifndef SIMCORE_BERTINI_LDMXCASCADEINTERFACE_H
#define SIMCORE_BERTINI_LDMXCASCADEINTERFACE_H

// Include the hack header first to expose private Geant4 members
#include "Framework/Logger.h"
#include "SimCore/Bertini/G4BertiniHack.h"
#include "SimCore/Event/CascadeHistory.h"

class G4HadProjectile;
class G4Nucleus;
class G4HadFinalState;

namespace simcore {
namespace bertini {

/**
 * @class LDMXCascadeInterface
 * @brief G4CascadeInterface that captures cascade history via preprocessor hack
 *
 * After ApplyYourself(), call extractHistory() to get the cascade in LDMX
 * format.
 */
class LDMXCascadeInterface : public G4CascadeInterface {
 public:
  LDMXCascadeInterface(const G4String& name = "LDMXBertiniCascade");
  virtual ~LDMXCascadeInterface();

  G4HadFinalState* ApplyYourself(const G4HadProjectile& projectile,
                                 G4Nucleus& targetNucleus) override;

  void setEnergyThreshold(double threshold) { energy_threshold_ = threshold; }
  double getEnergyThreshold() const { return energy_threshold_; }

  /** Returns nullptr if no history was captured */
  const ldmx::CascadeHistory* getLastCascadeHistory() const {
    return last_history_.empty() ? nullptr : &last_history_;
  }

  /** Move the captured history out */
  ldmx::CascadeHistory extractHistory() { return std::move(last_history_); }

  bool hasHistory() const { return !last_history_.empty(); }

  void setIncidentTrackId(int trackId) { incident_track_id_ = trackId; }

 private:
  /**
   * Ensure the G4CascadeHistory object exists in the cascader
   * Geant4 only creates this if G4CASCADE_SHOW_HISTORY envvar is set,
   * so we force-create it here to enable history capture.
   */
  void ensureCascadeHistoryExists();

  /**
   * Extract history from the internal G4CascadeHistory
   * Navigates: this->collider->theIntraNucleiCascader->theCascadeHistory
   */
  void captureHistory();

  /**
   * Capture de-excitation products from G4HadFinalState
   *
   * De-excitation (evaporation, gamma, fission) is handled by
   * G4ExcitationHandler after the cascade and isn't recorded in
   * G4CascadeHistory. Identifies them by comparing final state secondaries
   * against cascade escapees.
   */
  void captureDeexcitationProducts(G4HadFinalState* finalState);

  /** Minimum photon energy threshold for recording [MeV] */
  double energy_threshold_{5000.0};

  /** Track ID of incident particle */
  int incident_track_id_{-1};

  /** Captured history from last cascade */
  ldmx::CascadeHistory last_history_;

  enableLogging("LDMXCascadeInterface")
};

}  // namespace bertini
}  // namespace simcore

#endif  // SIMCORE_BERTINI_LDMXCASCADEINTERFACE_H
