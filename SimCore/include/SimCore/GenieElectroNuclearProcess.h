/**
 * @file GenieElectroNuclearProcess.h
 * @brief G4VDiscreteProcess that uses GENIE to simulate electronuclear
 *        interactions as the electron traverses material.
 */

#ifndef SIMCORE_GENIEELECTRONUCLEARPROCESS_H
#define SIMCORE_GENIEELECTRONUCLEARPROCESS_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Framework/Configure/Parameters.h"
#include "Framework/Logger.h"
#include "G4VDiscreteProcess.hh"
#include "GENIE/Framework/EventGen/GEVGDriver.h"
#include "GENIE/Framework/EventGen/HepMC3Converter.h"

class G4Element;

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

  /**
   * Parse the spline file for the set of target nuclei that have cross-section
   * splines available.  Used to skip discovered isotopes without splines, which
   * GENIE would otherwise compute on the fly (prohibitively slow).
   */
  void loadAvailableTargets();

  /**
   * @return true if a cross-section spline is available for @p target_code,
   *         or if the available-target list could not be determined (fail
   * open).
   */
  bool splineAvailable(int target_code) const;

  /// Set up GEVGDrivers for each target isotope (manual mode)
  void setupDrivers();

  /// Discover isotopes from a G4Element and set up GENIE drivers (auto mode)
  void discoverIsotopesForElement(const G4Element* element);

  /**
   * One-shot auto-discovery of target isotopes from the configured volume.
   *
   * Looks up every G4LogicalVolume matching @ref discover_volume_ (using the
   * same name/region tests as the ElectroNuclear bias operator), and sets up
   * GENIE drivers for the elements of their materials.  Called once on the
   * first GetMeanFreePath, after the geometry has been built.
   */
  void discoverFromVolume();

  /// One GENIE event generator driver per target isotope
  std::vector<std::unique_ptr<genie::GEVGDriver>> evg_drivers_;

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

  /// Whether targets are auto-discovered from the Geant4 geometry
  bool auto_discover_{false};

  /// Name of the volume to auto-discover targets from (e.g. "target_region").
  /// Uses the same naming convention as the ElectroNuclear bias operator.
  std::string discover_volume_;

  /// Z values already checked for isotope discovery
  std::set<int> discovered_z_;

  /// Target nuclei (10LZZZAAAI codes) that have splines in the spline file.
  /// Empty if the spline file could not be parsed (then no filtering is done).
  std::set<int> available_targets_;

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
