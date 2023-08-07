/**
 * @file GeneralParticleSource.h
 * @brief Extension of G4GeneralParticleSource.
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_GENERALPARTICLESOURCE_H
#define SIMCORE_GENERALPARTICLESOURCE_H

//------------//
//   Geant4   //
//------------//
#include "G4GeneralParticleSource.hh"

//------------//
//   LDMX     //
//------------//
#include "SimCore/PrimaryGenerator.h"

// Forward declarations
class G4Event;

namespace simcore {
namespace generators {

/**
 * @class GeneralParticleSource
 * @brief Class that extends the functionality of G4GeneralParticleSource.
 */
class GeneralParticleSource : public simcore::PrimaryGenerator {
 public:
  /**
   * Constructor.
   *
   * @param parameters Parameters used to configure the particle gun.
   *
   * Parameters:
   *  initCommands : vector of Geant4 strings to initialize the GPS
   */
  GeneralParticleSource(const std::string& name, const framework::config::Parameters& parameters);

  /// Destructor
  virtual ~GeneralParticleSource() = default;

  /**
   * Generate the primary vertices in the Geant4 event.
   *
   * @param event The Geant4 event.
   */
  void GeneratePrimaryVertex(G4Event* event) final override;
  void RecordConfig(const std::string& id, ldmx::RunHeader& rh) final override;

 private:
  /**
   * The underlying Geant4 GPS implementation.
   *
   * The creation of this class creates a new messenger that we can pass
   * commands to.
   */
  G4GeneralParticleSource theG4Source_;

  /// storage of initialization commands (for config recording)
  std::vector<std::string> init_commands_;

};  // GeneralParticleSource

}  // namesapce generators
}  // namespace simcore

#endif  // SIMCORE_GENERALPARTICLESOURCE_H
