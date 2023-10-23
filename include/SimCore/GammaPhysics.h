/**
 * @file GammaPhysics.h
 * @brief Class used to enhanced the gamma physics list.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Einar Elen, Lund University
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_GAMMAPHYSICS_H_
#define SIMCORE_GAMMAPHYSICS_H_

//------------//
//   Geant4   //
//------------//
#include "G4GammaConversionToMuons.hh"
#include "G4ProcessManager.hh"
#include "G4VPhysicsConstructor.hh"
#include "G4VProcess.hh"
#include "SimCore/PhotoNuclearModel.h"

namespace simcore {

/**
 * @class GammaPhysics @brief Adds extra gamma particle physics for simulation
 * and sets up the photonuclear model to use from the configuration
 *
 * @note
 *
 * Is responsible for selecting the photonuclear model from the python
 * configuration.
 * Currently adds gamma -> mumu reaction using the
 * <i>G4GammaConversionToMuons</i> process. Also changes ordering of
 * gamma processes such that photonNuclear and GammaToMuMu are called first.
 */
class GammaPhysics : public G4VPhysicsConstructor {
 public:
  /**
   * Class constructor.
   * @param name The name of the physics.
   * @param parameters The python configuration
   */
  GammaPhysics(const G4String& name,
               const framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  virtual ~GammaPhysics() = default;

  /**
   * Construct particles
   *
   * We don't do anything here since we are just attaching/updating
   * the photon physics.
   */
  void ConstructParticle() final override;

  /**
   * We do two things for this call back during initialization.
   *
   * 1. We add the muon-conversion process to the photon's process
   *    table, enabling it to be one of the processes that can happen.
   * 2. We move the photonNuclear process to be the first process in
   *    the photon's ordering so that the bias operator has a chance
   *    to lower its cross-section before any EM process is called
   *    (if need be).
   */
  void ConstructProcess() final override;

 private:
  /**
   * The gamma to muons process.
   */
  G4GammaConversionToMuons gammaConvProcess;
  /**
   * Parameters from the configuration to pass along to the photonuclear model.
   */
  framework::config::Parameters modelParameters;
};

}  // namespace simcore

#endif
