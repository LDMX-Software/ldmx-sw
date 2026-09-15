/**
 * @file HepMCPrimaryGenerator.h
 * @brief Class for generating a Geant4 event from HepMC event data
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_HEPMCPRIMARYGENERATOR_H
#define SIMCORE_HEPMCPRIMARYGENERATOR_H

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"
#include "SimCore/G4User/UserPrimaryParticleInformation.h"
#include "SimCore/Generators/PrimaryGenerator.h"
#include "SimCore/HepMC/HepMCReader.h"

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
 * @class HepMCPrimaryGenerator
 * @brief Generates a Geant4 event from a HepMCEvent
 */
class HepMCPrimaryGenerator : public simcore::PrimaryGenerator {
 public:
  /**
   * Class constructor.
   * @param name The name of the generator.
   * @param parameters Configuration parameters.
   */
  HepMCPrimaryGenerator(const std::string& name,
                        const framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  virtual ~HepMCPrimaryGenerator() = default;

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
   * The file path to the HepMC file.
   */
  std::string file_path_;

  /**
   * The HepMC reader with the event data
   */
  hepmc::HepMCReader reader_;

  /**
   * The vertex offset to apply to the HepMC event vertex.
   */
  std::vector<double> vertex_;

  // enable logging
  enableLogging("HepMCPrimaryGenerator")
};

}  // namespace generators
}  // namespace simcore

#endif  // SIMCORE_HEPMCPRIMARYGENERATOR_H
