/**
 * @file G4eDarkBremsstrahlung.h
 * @brief Class providing the Dark Bremsstrahlung process class.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_DARKBREM_G4EDARKBREMSSTRAHLUNG_H_
#define SIMCORE_DARKBREM_G4EDARKBREMSSTRAHLUNG_H_

#include "Framework/Configure/Parameters.h"
#include "Framework/Logger.h"
#include "Framework/RunHeader.h"

// Geant
#include "G4VDiscreteProcess.hh"

class G4String;
class G4ParticleDefinition;

namespace simcore {
namespace darkbrem {

/**
 * @class G4eDarkBremmsstrahlungModel
 * Abstract class representing a model for dark brem.
 *
 * The model is what actually determines two important things:
 *  1. How the cross section is calculated
 *  2. What the particle change is when the process happens
 *
 * This class is the base class that shows what is necessary
 * for the model to function properly.
 */
class G4eDarkBremsstrahlungModel {
 public:
  /**
   * Constructor
   *
   * Configures the model based on the passed parameters
   *
   * Names the logger after the name for this model.
   */
  G4eDarkBremsstrahlungModel(const framework::config::Parameters& p) {
    theLog_ =
        framework::logging::makeLogger(p.getParameter<std::string>("name"));
  }

  /// Destructor, nothing on purpose
  virtual ~G4eDarkBremsstrahlungModel() = default;

  /**
   * Print the configuration of this model
   *
   * Helpful for debugging and keeping the process compliant
   * with the other Geant4 processes.
   */
  virtual void PrintInfo() const = 0;

  /**
   * Record the configuration of this model to the RunHeader
   *
   * Helpful for persisting run data for later viewing.
   */
  virtual void RecordConfig(ldmx::RunHeader& h) const = 0;

  /**
   * Calculate the cross section given the input parameters
   *
   * @see G4eDarkBremmstrahlung::GetMeanFreePath
   * @param[in] electronKE current electron kinetic energy
   * @param[in] atomicA atomic-mass number for the element the electron is in
   * @param[in] atomicZ atomic-number for the element the electron is in
   * @returns cross section with units incorporated as a G4double
   */
  virtual G4double ComputeCrossSectionPerAtom(G4double electronKE,
                                              G4double atomicA,
                                              G4double atomicZ) = 0;

  /**
   * Generate the change in the particle now that we can assume the interaction
   * is occuring
   *
   * @note The input particleChange has already been cleared and then
   * initialized, so there is no need for the model to do those steps.
   *
   * @see G4eDarkBremmstrahlung::PostStepDoIt
   * @param[in,out] particleChange particle change class that stores information
   * @param[in] track current track that needs the change
   * @param[in] step current step of the track
   */
  virtual void GenerateChange(G4ParticleChange& particleChange,
                              const G4Track& track, const G4Step& step) = 0;

 protected:
  /// The logging apparatus for this model
  framework::logging::logger theLog_;

};  // G4eDarkBremsstrahlungModel

/**
 * The cache of already computed cross sections
 *
 * We make a specific class for the cache in order
 * to keep the key encoding/decoding process in a central
 * location.
 */
class ElementXsecCache {
 public:
  /**
   * Default constructor
   *
   * Does nothing interesting, but no model for calculating cross section has
   * been set.
   */
  ElementXsecCache() = default;

  /**
   * Constructor with a model to calculate the cross section.
   */
  ElementXsecCache(std::shared_ptr<G4eDarkBremsstrahlungModel> model)
      : model_{model} {}

  /**
   * Get the value of the cross section for the input variables
   * and calculate the cross section if it wasn't calculated before.
   *
   * @raises Exception if no model is available for calculating cross sections
   * @param[in] energy Energy of incident electron [MeV]
   * @param[in] A atomic mass of element [atomic mass units]
   * @param[in] Z atomic number of element [num protons]
   * @returns cross section corresponding to the input parameters (including
   * units Geant4 style)
   */
  G4double get(G4double energy, G4double A, G4double Z);

  /**
   * Stream the entire table into the output stream.
   *
   * @param[in,out] o ostream to write to
   */
  void stream(std::ostream& o) const;

  /**
   * Overload the streaming operator for ease
   *
   * @param[in] o ostream to write to
   * @param[in] c cache to write out
   * @returns modified ostream
   */
  friend std::ostream& operator<<(std::ostream& o, const ElementXsecCache c) {
    c.stream(o);
    return o;
  }

 private:
  /// The type for the key we use in the cache
  typedef unsigned long int key_t;

  /// The maximum value of A
  static const key_t MAX_A{1000};

  /// The maximum value for energy [MeV]
  static const key_t MAX_E{30000};

  /**
   * Compute a key for the cache map
   * Generating a unique key _after_ making the energy [MeV] an integer.
   * The atomic mass (A) and charge (Z) are given by Geant4 as doubles as well,
   * so I cast them to integers before computing the key.
   *
   * This is what you would edit if you want a more/less find-grained cache
   * of Xsecs. Right now, since the internal unit of energy in Geant4 is MeV,
   * the cache is binned at the 1MeV scale.
   *
   * @param[in] energy Energy of incident electron [MeV]
   * @param[in] A atomic mass of element [atomic mass units]
   * @param[in] Z atomic number of element [num protons]
   * @returns unsigned integer cache key for these three inputs
   */
  key_t computeKey(G4double energy, G4double A, G4double Z) const;

 private:
  /// the actual map from cache keys to calculated cross sections
  std::map<key_t, G4double> the_cache_;

  /// shared pointer to the model for calculating cross sections
  std::shared_ptr<G4eDarkBremsstrahlungModel> model_;

};  // ElementXsecCache

/**
 * @class G4eDarkBremsstrahlung
 *
 * Class that represents the dark brem process.
 * An electron is allowed to brem a dark photon
 *
 * @TODO allow positrons to dark brem as well
 */
class G4eDarkBremsstrahlung : public G4VDiscreteProcess {
 public:
  /**
   * The name of this process in Geant4
   *
   * @note This process name should be used in all places that
   * can depend on this file. (For example RunManager and DetectorConstruction).
   * There are other places that _can't_ use this constant
   * directly, so if this name is changed you also need to change
   * the following places.
   *  - Python: Biasing.filters.TrackProcessFilter.dark_brem
   *  - C++: Event/SimParticle::createProcessMap
   */
  static const std::string PROCESS_NAME;

  /**
   * Constructor
   *
   * Configures this process by doing three main things:
   *  1. Registers this process with Geant4 as a 'fElectromagnetic' process
   *     - Needed for Geant4 Biasing Framework to recognize this process as
   * "bias-able"
   *  2. Defines the EM subtype as one different from all other EM processes
   *     - Needed so we don't replace another EM process
   *  3. Configures the process and passes the model parameters to the model
   *
   * If caching the cross section is enabled, we calculate several
   * common cross sections immediately to help even out the time
   * it takes to simulate events.
   * @see CalculateCommonXsec
   */
  G4eDarkBremsstrahlung(const framework::config::Parameters& params);

  /**
   * Destructor
   */
  virtual ~G4eDarkBremsstrahlung() = default;

  /**
   * Checks if the passed particle should be able to do this process
   *
   * @return true if particle is electron
   */
  virtual G4bool IsApplicable(const G4ParticleDefinition& p);

  /**
   * Reports the parameters to G4cout.
   *
   * @see G4eDarkBremsstrahlungModel::PrintInfo
   */
  virtual void PrintInfo();

  /**
   * Records the configuration of this process into the RunHeader
   *
   * @see G4eDarkBremsstrahlungModel::RecordConfig
   * @param[in,out] h RunHeader to write to
   */
  void RecordConfig(ldmx::RunHeader& h) const;

  /**
   * This is the function actually called by Geant4 that does the dark brem
   * interaction.
   *
   * aParticleChange is a protected member variable of G4VDiscreteProcess
   * that we should edit here.
   *
   * If only one per event is set, then we deactivate the dark brem process,
   * ensuring only one dark brem per step and per event.
   * Reactivated in RunManager::TerminateOneEvent.
   *
   * @see RunManager::TerminateOneEvent
   * @see G4eDarkBremsstrahlungModel::GenerateChange
   * @param[in] track current G4Track that is being stepped
   * @param[in] step current step that just finished
   * @returns G4VParticleChange detailing how this process changes the track
   */
  virtual G4VParticleChange* PostStepDoIt(const G4Track& track,
                                          const G4Step& step);

  /**
   * Calculate common cross sections for the cache using the already-created
   * model.
   *
   * This method is public so that we can access it for writing a short
   * executable to print the cache to a file.
   */
  void CalculateCommonXsec();

  /**
   * Get a reference to the cross section cache.
   *
   * Again, this method is public only to be available to the executable
   * that generates a cross section table and testing.
   * Do not use this unless you really know what you are doing.
   */
  ElementXsecCache& getCache() { return element_xsec_cache_; }

 protected:
  /**
   * Calculate the mean free path given the input conditions
   *
   * We maintain a cache for the cross sections calculated by the model
   * so that later in the run it is less likely that the model will
   * need to be called to calculate the cross section. This is done
   * in order to attempt to improve speed of simulation and avoid
   * repetition of the same, deterministic calculations.
   *
   * If you want to turn off the cache-ing behavior, set 'cache_xsec' to false
   * in the python configuratin for the dark brem process.
   *
   * @see G4eDarkBremsstrahlungModel::ComputeCrossSectionPerAtom
   * @param[in] track G4Track that is being stepped
   * @param[in] prevStepSize G4double measuring previous step size, unused
   * @param[in] condition G4ForceCondition, always NotForced for
   * G4VDiscreteProcess, unused
   * @returns G4double mean free path of the particle
   */
  G4double GetMeanFreePath(const G4Track& track, G4double prevStepSize,
                           G4ForceCondition* condition);

 private:
  /** remove ability to assign this object */
  G4eDarkBremsstrahlung& operator=(const G4eDarkBremsstrahlung& right);

  /** remove ability to copy construct */
  G4eDarkBremsstrahlung(const G4eDarkBremsstrahlung&);

  /**
   * Only allow the dark brem to happen once per event.
   *
   * This allows for the dark brem process to be de-activated when
   * SampleSecondaries is called.
   *
   * The dark brem process is _always_ re-activated in the
   * RunManager::TerminateOneEvent method. This reactivation has no effect when
   * the process is already active.
   */
  bool only_one_per_event_;

  /**
   * The mass of the A' during this run [MeV]
   */
  double ap_mass_;

  /**
   * Should we have a cache for the computed cross sections?
   */
  bool cache_xsec_;

  /**
   * The model that we are using in this run.
   *
   * Shared with the chaching class.
   */
  std::shared_ptr<G4eDarkBremsstrahlungModel> model_;

  /// Our instance of a cross section cache
  ElementXsecCache element_xsec_cache_;

  /// Enable logging for this process
  framework::logging::logger theLog_ =
      framework::logging::makeLogger("DarkBremProcess");

};  // G4eDarkBremsstrahlung

}  // namespace darkbrem
}  // namespace simcore

#endif  // SIMCORE_DARKBREM_G4EDARKBREMSSTRAHLUNG_H_
