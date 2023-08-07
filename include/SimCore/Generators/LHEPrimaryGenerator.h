/**
 * @file LHEPrimaryGenerator.h
 * @brief Class for generating a Geant4 event from LHE event data
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_LHEPRIMARYGENERATOR_H
#define SIMCORE_LHEPRIMARYGENERATOR_H

// LDMX
#include "SimCore/LHE/LHEReader.h"
#include "SimCore/PrimaryGenerator.h"

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
   * @param reader The LHE reader with the event data.
   */
  LHEPrimaryGenerator(const std::string& name, const framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  virtual ~LHEPrimaryGenerator();

  /**
   * Generate vertices in the Geant4 event.
   * @param anEvent The Geant4 event.
   */
  void GeneratePrimaryVertex(G4Event* anEvent) final override;

  void RecordConfig(const std::string& id, ldmx::RunHeader& rh) final override;

 private:
  /**
   * The LHE reader with the event data.
   */
  simcore::lhe::LHEReader* reader_;
  /// path to LHE file
  std::string file_path_;
};

}  // namespace generators
}  // namespace simcore

#endif  // SIMCORE_LHEPRIMARYGENERATOR_H
