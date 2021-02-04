/**
 * @file RunManager.h
 * @brief Class providing a Geant4 run manager implementation.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_RUNMANAGER_H
#define SIMCORE_RUNMANAGER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <map>
#include <string>

//------------//
//   Geant4   //
//------------//
#include "G4PhysListFactory.hh"
#include "G4RunManager.hh"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"

namespace simcore {

// Forward declare to avoid circular dependency in headers
class DetectorConstruction;
class UserActionManager;
class APrimeMessenger;
class ConditionsInterface;

/**
 * @class RunManager
 * @brief Extension of Geant4 run manager
 */
class RunManager : public G4RunManager {
 public:
  /**
   * Class constructor.
   */
  RunManager(framework::config::Parameters& parameters,
             ConditionsInterface& ci);

  /**
   * Class destructor.
   */
  virtual ~RunManager();

  /**
   * Initialize physics.
   */
  void setupPhysics();

  /**
   * Perform application initialization.
   */
  void Initialize();

  /**
   * Called at the end of each event.
   *
   * Runs parent process G4RunManager::TerminateOneEvent() and
   * resets the activation for the G4eDarkBremsstrahlung process
   * (if dark brem is possible)
   */
  void TerminateOneEvent();

  /**
   * Get the user detector construction cast to a specific type.
   * @return The user detector construction.
   */
  DetectorConstruction* getDetectorConstruction();

  /**
   * Tell RunManager to use the seed from the root file.
   */
  void setUseRootSeed(bool useIt = true) { useRootSeed_ = useIt; }

  /**
   * Should we use the seed from the root file?
   */
  bool useRootSeed() { return useRootSeed_; }

 private:
  /// The set of parameters used to configure the RunManager
  framework::config::Parameters parameters_;

  /**
   * Factory class for instantiating the physics list.
   */
  G4PhysListFactory physicsListFactory_;

  /**
   * Flag indicating whether a parallel world should be
   * registered
   */
  bool isPWEnabled_{false};

  /** Path to GDML description of parallel world. */
  std::string parallelWorldPath_{""};

  /**
   * Should we use random seed from root file?
   */
  bool useRootSeed_{false};

  /** ConditionsInterface
   */
  ConditionsInterface& conditionsIntf_;

};  // RunManager
}  // namespace simcore

#endif  // _SIMCORE_RUNMANAGER_H_
