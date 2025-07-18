/**
 * @file LHEPrimaryGenerator.cxx
 * @brief Implementation file for LHEPrimaryGenerator
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_LHEPRIMARYGENERATOR_H
#define SIMCORE_LHEPRIMARYGENERATOR_H

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"
#include "SimCore/G4User/UserPrimaryParticleInformation.h"
#include "SimCore/Generators/PrimaryGenerator.h"
#include "SimCore/LHE/LHEReader.h"

// Geant4
#include "G4Event.hh"
#include "G4IonTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"

// LDMX

class G4Event;

namespace simcore {
namespace generators {

/**
 * @class LHEPrimaryGenerator
 * @brief Generates a Geant4 event from an LHEEvent
 */
class LHEPrimaryGenerator : public simcore::PrimaryGenerator {
 public:
  /**
   * Class constructor.
   * @param name The name of the generator.
   * @param parameters Configuration parameters.
   */
  LHEPrimaryGenerator(const std::string& name,
                      const framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  virtual ~LHEPrimaryGenerator() = default;

  /**
   * Generate vertices in the Geant4 event.
   * @param anEvent The Geant4 event.
   */
  void GeneratePrimaryVertex(G4Event* anEvent) override;

  /**
   * Record configuration information.
   * @param id The configuration ID.
   * @param rh The run header.
   */
  void RecordConfig(const std::string& id, ldmx::RunHeader& rh) override;

 private:
  /**
   * The LHE reader with the event data, managed by a smart pointer.
   */
  std::unique_ptr<simcore::lhe::LHEReader> reader_;

  /// Path to the LHE file.
  std::string file_path_;

  // enable logging
  enableLogging("LHEPrimaryGenerator")
};

}  // namespace generators
}  // namespace simcore

#endif  // SIMCORE_LHEPRIMARYGENERATOR_H