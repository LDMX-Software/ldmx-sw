/**
 * @file LHEPrimaryGenerator
 * @brief Class for generating a Geant4 event from LHE event data
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
  void recordConfig(const std::string& id, ldmx::RunHeader& rh) override;

 private:
  /**
   * The file path to the LHE file.
   */
  std::string file_path_;

  /**
   * The LHE reader with the event data
   */

  lhe::LHEReader reader_;

  /**
   * The vertex offset to apply to the LHE event vertex.
   */
  std::vector<double> vertex_;

  // enable logging
  enableLogging("LHEPrimaryGenerator")
};

}  // namespace generators
}  // namespace simcore

#endif  // SIMCORE_LHEPRIMARYGENERATOR_H
