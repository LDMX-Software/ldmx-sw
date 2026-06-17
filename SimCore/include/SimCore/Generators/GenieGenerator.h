/**
 * @file GenieGenerator.h
 * @brief Simple GENIE event generator.
 * @author Wesley Ketchum, FNAL
 */

#ifndef SIMCORE_GENIE_GENERATOR_H
#define SIMCORE_GENIE_GENERATOR_H

//----------//
//   ROOT   //
//----------//
#include "TRandom.h"
#include "TRandomGen.h"

//------------//
//   GENIE   //
//------------//
#include <string>
#include <vector>

#include "GENIE/Framework/EventGen/GEVGDriver.h"
#include "GENIE/Framework/EventGen/HepMC3Converter.h"

//------------//
//   LDMX     //
//------------//
#include "Framework/Logger.h"
#include "SimCore/G4User/UserEventInformation.h"
#include "SimCore/Generators/PrimaryGenerator.h"

// Forward declarations
class G4Event;

namespace simcore {
namespace generators {

/**
 * @class GenieGenerator
 * @brief Class that uses GENIE's GEVGDriver to generator eN interactions.
 */
class GenieGenerator : public simcore::PrimaryGenerator {
 public:
  /**
   * Constructor.
   *
   * @param parameters Parameters used to configure GENIE generator.
   *
   * Parameters:
   *  energy    : energy of initial electron (GeV)
   *  targets   : list of ten-digit 10LZZZAAAI target codes
   *  abundances: list of relative abundances for the given targets
   *  position  : position of interaction from (mm three-vector)
   *  time      : time to shoot at (ns)
   *  direction : direction to shoot in (unitless three-vector)
   *  tune      : name of GENIE tune
   *  seed      : seed for random generator
   */
  GenieGenerator(const std::string& name,
                 const framework::config::Parameters& parameters);

  /// Destructor
  ~GenieGenerator();

  /**
   * Generate the primary vertices in the Geant4 event.
   *
   * @param event The Geant4 event.
   */
  void GeneratePrimaryVertex(G4Event* event) final override;

  void RecordConfig(const std::string& id, ldmx::RunHeader& rh) final override;

 private:
  /**
   * One GENIE event generator driver per target and convertor to
   * HepMC3GenEvent. Each driver is configured once for its target during
   * initialization.
   */
  std::vector<genie::GEVGDriver> evg_drivers_;
  genie::HepMC3Converter hep_mc3_converter_;

  double energy_;
  std::vector<int> targets_;
  std::vector<double> abundances_;
  std::vector<double> position_;
  std::vector<double> beam_size_;
  float target_thickness_;
  float time_;
  std::vector<double> direction_;

  std::string tune_;
  std::string spline_file_;

  std::string message_threshold_file_;

  /// Whether to include an upstream beam electron primary
  bool include_beam_electron_;
  /// Starting position of the beam electron [mm]
  std::vector<double> beam_electron_position_;
  /// Unit direction of the beam electron
  std::vector<double> beam_electron_direction_;
  /// Energy of the beam electron [GeV]
  double beam_electron_energy_;

  std::vector<float> ev_weighting_integral_;
  size_t n_events_generated_;
  std::vector<size_t> n_events_by_target_;
  std::vector<float> xsec_by_target_;

  float xsec_total_;

  void fillConfig(
      const framework::config::Parameters&);  /// fill the configuration
  bool validateConfig();  /// simple validation check on configuration params

  void initializeGENIE();   /// GENIE initialization
  void calculateTotalXS();  /// GENIE initialization

  enableLogging("GenieGenerator")

};  // GenieGenerator

}  // namespace generators
}  // namespace simcore

#endif  // SIMCORE_GENIE_GENERATOR_H
