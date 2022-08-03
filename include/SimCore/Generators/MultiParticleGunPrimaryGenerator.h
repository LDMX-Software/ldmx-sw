/**
 * @file MultiParticleGunPrimaryGenerator.h
 * @brief Class for generating an event using multiple particles.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Nhan Tran, FNAL
 */

#ifndef SIMCORE_MULTIPARTICLEGUNPRIMARYGENERATOR_H_
#define SIMCORE_MULTIPARTICLEGUNPRIMARYGENERATOR_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <fstream>
#include <iostream>
#include <string>

//------------//
//   Geant4   //
//------------//
#include "G4Event.hh"
#include "G4IonTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPrimaryGenerator.hh"

//----------//
//   ROOT   //
//----------//
#include "TFile.h"
#include "TLorentzVector.h"
#include "TRandom.h"
#include "TTree.h"
#include "TVector3.h"

//-------------//
//   LDMX-SW   //
//-------------//
#include "Framework/EventHeader.h"
#include "SimCore/PrimaryGenerator.h"
#include "SimCore/UserPrimaryParticleInformation.h"

namespace simcore {
namespace generators {

/**
 * @class MultiParticleGunPrimaryGenerator
 * @brief Generates a Geant4 event from particle gun, but can have many
 * particles
 */
class MultiParticleGunPrimaryGenerator : public simcore::PrimaryGenerator {
 public:
  /**
   * Constructor
   *
   * @param name the name of this generator
   * @param parameters the configuration parameters
   *
   * Parameters:
   *  vertex        : Position to shoot from (mm)
   *  momentum      : 3-vector mometum of particles (MeV)
   *  nParticles    : number of particles to shoot (mean if Poisson enabled)
   *  pdgID         : pdgID of particle to shoot
   *  enablePoisson : whether to poisson distribute the number of particles
   */
  MultiParticleGunPrimaryGenerator(const std::string& name,
                                   const framework::config::Parameters& parameters);

  /** Destructor */
  virtual ~MultiParticleGunPrimaryGenerator();

  /**
   * Generate vertices in the Geant4 event.
   *
   * @param anEvent The Geant4 event.
   */
  void GeneratePrimaryVertex(G4Event* anEvent);

  void RecordConfig(const std::string& id, ldmx::RunHeader& rh) final override;
 private:
  /** Random number generator. */
  TRandom* random_;

  /** The vertex position from which to fire the particles. */
  G4ThreeVector mpgVertex_;

  /** The initial momentum of the particles. */
  G4ThreeVector mpgMomentum_;

  /** Number of particles that will be fired by the gun per event. */
  double mpgNParticles_{1.};

  /** PDG ID of the particle used by the gun. */
  int mpgPdgID_{99999};

  /**
   * Flag denoting whether the number of incident particles should
   * be Poisson distributed.
   */
  bool mpgEnablePoisson_{false};

};  // MultiParticleGunPrimaryGenerator

}  // namespace generators
}  // namespace simcore

#endif  // SIMCORE_MULTIPARTICLEGUNPRIMARYGENERATOR_H_
