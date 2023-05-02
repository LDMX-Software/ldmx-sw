/**
 * @file GammaPhysics.h
 * @brief Class used to enhanced the gamma physics list.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_GAMMAPHYSICS_H_
#define SIMCORE_GAMMAPHYSICS_H 1_

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
               const framework::config::Parameters& parameters)
      : G4VPhysicsConstructor(name),
        modelParameters{parameters.getParameter<framework::config::Parameters>(
            "photonuclear_model")} {}
  GammaPhysics(const G4String& name = "GammaPhysics");

  /**
   * Class destructor.
   */
  virtual ~GammaPhysics();

  /**
   * Construct particles (no-op).
   */
  void ConstructParticle() {}

  /**
   * Construct the process (gamma to muons).
   */
  void ConstructProcess();

  /**
   * Search the list for the process "photoNuclear".  When found,
   * change the calling order so photonNuclear is called before
   * EM processes. The biasing operator needs the photonNuclear
   * process to be called first because the cross-section is
   * needed to bias down other process.
   */
  void SetPhotonNuclearAsFirstProcess() const;

 private:
  /*
   * Returns the process manager object for the G4Gamma class from the list of
   * Geant4 particles.
   */
  G4ProcessManager* GetGammaProcessManager() const;
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
