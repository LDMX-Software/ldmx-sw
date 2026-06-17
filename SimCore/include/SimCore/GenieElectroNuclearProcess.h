/**
 * @file GenieElectroNuclearProcess.h
 * @brief G4VDiscreteProcess that uses GENIE to simulate electronuclear
 *        interactions as the electron traverses material.
 */

#ifndef SIMCORE_GENIEELECTRONUCLEARPROCESS_H
#define SIMCORE_GENIEELECTRONUCLEARPROCESS_H

#include <map>
#include <string>
#include <vector>

#include "Framework/Configure/Parameters.h"
#include "Framework/Logger.h"
#include "G4VDiscreteProcess.hh"
#include "GENIE/Framework/EventGen/GEVGDriver.h"
#include "GENIE/Framework/EventGen/HepMC3Converter.h"

namespace simcore {

/**
 * @class GenieElectroNuclearProcess
 * @brief Geant4 discrete process that fires GENIE electronuclear interactions.
 *
 * Replaces the built-in Geant4 "electronNuclear" process so that the existing
 * ElectroNuclear bias operator works without modification.  Follows the same
 * architectural pattern as G4DarkBremsstrahlung.
 *
 * The electron is tracked normally through upstream material (tagger, etc.)
 * with natural energy loss.  When Geant4 selects this process to fire, GENIE
 * generates the interaction using the electron's actual energy at that point.
 * The GENIE final-state particles are injected as Geant4 secondaries and the
 * primary electron is killed.
 */
class GenieElectroNuclearProcess : public G4VDiscreteProcess {
 public:
  /// Process name — matches the built-in so the bias operator works unchanged
  static const std::string PROCESS_NAME;

  /**
   * Constructor.
   *
   * Initializes GENIE (tune, splines, event generator drivers) and builds
   * the Z-to-target lookup map for fast material matching.
   *
   * @param params Configuration parameters (targets, abundances, tune, etc.)
   */
  GenieElectroNuclearProcess(const framework::config::Parameters& params);

  /// Destructor
  ~GenieElectroNuclearProcess() override;

  /**
   * @return true if the particle is an electron
   */
  G4bool IsApplicable(const G4ParticleDefinition& p) override;

  /**
   * Called by Geant4 when this process fires.
   *
   * Generates a GENIE event at the electron's current energy, injects
   * final-state particles as secondaries, stores the HepMC3 record, and
   * kills the primary electron.
   */
  G4VParticleChange* PostStepDoIt(const G4Track& track,
                                  const G4Step& step) override;

  /// Set whether to allow only one interaction per event
  void setOnlyOnePerEvent(bool val) { only_one_per_event_ = val; }

 protected:
  /**
   * Calculate the mean free path for the electronuclear process.
   *
   * Loops over elements in the current material, looks up matching GENIE
   * targets by Z, queries GENIE for the cross section, and returns 1/SIGMA.
   */
  G4double GetMeanFreePath(const G4Track& track, G4double prevStepSize,
                           G4ForceCondition* condition) override;

 private:
  /// Initialize the GENIE framework (tune, message thresholds, splines)
  void initializeGENIE();

  /// Set up GEVGDrivers for each target isotope
  void setupDrivers();

  /// One GENIE event generator driver per target isotope
  std::vector<genie::GEVGDriver> evg_drivers_;

  /// Converter from GENIE EventRecord to HepMC3
  genie::HepMC3Converter hep_mc3_converter_;

  /// GENIE target codes (10LZZZAAAI format)
  std::vector<int> targets_;

  /// Relative abundances for each target
  std::vector<double> abundances_;

  /// GENIE tune name
  std::string tune_;

  /// Path to GENIE cross-section spline file
  std::string spline_file_;

  /// Path to GENIE message threshold configuration
  std::string message_threshold_file_;

  /// Only allow one EN interaction per event (then deactivate)
  bool only_one_per_event_{true};

  /// Flag to lazily sync GENIE random seed on first event
  bool genie_initialized_{false};

  /**
   * Lookup map: element Z -> list of (driver_index, abundance).
   * Built at construction time for fast material matching in
   * GetMeanFreePath.
   */
  std::map<int, std::vector<std::pair<int, double>>> z_to_targets_;

  /**
   * Partial cross-section sums per element, used to select the target
   * isotope in PostStepDoIt via weighted random sampling.
   * Indexed the same as the material's element vector.
   */
  std::vector<double> partial_sum_sigma_;

  enableLogging("GenieElectroNuclearProcess")
};

}  // namespace simcore

#endif  // SIMCORE_GENIEELECTRONUCLEARPROCESS_H
