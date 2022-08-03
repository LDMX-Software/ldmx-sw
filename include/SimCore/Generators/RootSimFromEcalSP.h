/**
 * @file RootSimFromEcalSP.h
 * @brief Primary generator used to generate primaries from SimParticles.
 * @author Nhan Tran, Fermilab
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_ROOTSIMFROMECALSP_H
#define SIMCORE_ROOTSIMFROMECALSP_H

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
 * @class RootSimFromEcalSP
 *
 * Generate primaries that correspond to particles leaving
 * the ECal (passing through the Ecal Scoring Planes) before
 * a time cutoff.
 */
class RootSimFromEcalSP : public PrimaryGenerator {
 public:
  /**
   * Class constructor.
   * @param name name of this generator
   * @param parameters configuration parameters
   *
   * Parameters:
   *   filePath : path to root file containing events to re-sim
   *   timeCutoff : maximum time that a particle can pass through Ecal Scoring
   * Planes and be included in re-sim (ns) ecalSPHitsCollName : name of
   * collection for Ecal Scoring Plane hits ecalSPHitsPassName : name of pass
   * for Ecal Scoring Plane hits
   */
  RootSimFromEcalSP(const std::string& name, const framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  virtual ~RootSimFromEcalSP();

  /**
   * Generate vertices in the Geant4 event.
   * @param anEvent The Geant4 event.
   */
  void GeneratePrimaryVertex(G4Event* anEvent);

  void RecordConfig(const std::string& id, ldmx::RunHeader& rh) final override;
 private:
  /**
   * The cutoff time
   *
   * Any particle passing through the ECal scoring planes at a time
   * greater than this time is NOT re-used as a primary.
   */
  double timeCutoff_;

  /**
   * The Ecal Scoring Planes Hits collection name
   */
  std::string ecalSPHitsCollName_;

  /**
   * The Ecal Scoring Planes Hits pass name
   */
  std::string ecalSPHitsPassName_;

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

#endif  // SIMCORE_ROOTSIMFROMECALSP_H
