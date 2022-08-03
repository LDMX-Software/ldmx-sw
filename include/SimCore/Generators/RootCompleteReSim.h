/**
 * @file RootCompleteReSim.h
 * @brief Primary generator used to generate primaries from SimParticles.
 * @author Nhan Tran, Fermilab
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_ROOTCOMPLETERESIM_H
#define SIMCORE_ROOTCOMPLETERESIM_H

//----------------//
//   C++ StdLib   //
//----------------//
#include <fstream>
#include <iostream>
#include <vector>

//----------//
//   ROOT   //
//----------//
#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "Framework/Event.h"
#include "Framework/EventFile.h"
#include "SimCore/PrimaryGenerator.h"

class G4Event;

namespace simcore {
namespace generators {

/**
 * @class RootCompleteReSim
 *
 * PrimaryGenerator that gets primaries and event seeds and
 * inputs them into current event as primaries with the exact same kinematics.
 */
class RootCompleteReSim : public simcore::PrimaryGenerator {
 public:
  /**
   * Class constructor.
   * @param name the name of the generator
   * @param parameters configuration parameters
   *
   * Parameters:
   *   filePath : path to root file to re-sim
   *   simParticleCollName : name of collection of SimParticles
   *   simParticlePassName : name of pass of SimParticles
   */
  RootCompleteReSim(const std::string& name, const framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  virtual ~RootCompleteReSim();

  /**
   * Generate vertices in the Geant4 event.
   * @param anEvent The Geant4 event.
   */
  void GeneratePrimaryVertex(G4Event* anEvent);

 private:
  /**
   * Name of SimParticles collection
   */
  std::string simParticleCollName_;

  /**
   * Name of SimParticles pass
   */
  std::string simParticlePassName_;

  /**
   * The input root file
   */
  std::unique_ptr<framework::EventFile> ifile_;

  /**
   * The input ldmx event bus
   */
  framework::Event ievent_;
};

}  // namespace generators
}  // namespace simcore

#endif  // SIMCORE_ROOTCOMPLETERESIM_H
