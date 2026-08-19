/**
 * @file FromScoringPlane
 * @brief Copy particles from a particular scoring plane in as primaries
 * of a new simulation
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_FROMSCORINGPLANE_H
#define SIMCORE_FROMSCORINGPLANE_H

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/Exception/Exception.h"
#include "Framework/Logger.h"
#include "SimCore/G4User/UserPrimaryParticleInformation.h"
#include "SimCore/Generators/PrimaryGenerator.h"

class G4Event;

namespace simcore {
namespace generators {

/**
 * @class FromScoringPlane
 * @brief Copy particles from a particular scoring plane in as primaries
 * of a new simulation
 */
class FromScoringPlane : public simcore::PrimaryGenerator {
 public:
  /**
   * Class constructor.
   * @param name The name of the generator.
   * @param parameters Configuration parameters.
   */
  FromScoringPlane(const std::string& name,
                   const framework::config::Parameters& parameters);

  /**
   * Class destructor.
   */
  virtual ~FromScoringPlane() = default;

  /**
   * Retrieve the collection of scoring plane hits and copy
   * the selected hits into our copies of primary particles.
   * The particles are copied into the G4Event in GeneratePrimaryVertex
   *
   * @throws framework::Exception if the requested scoring plane hits
   * are not available, most often because the input file containing
   * them was not requested
   *
   * @param[in] event framework::Event containing necessary scoring plane hits
   */
  void PrepEvent(const framework::Event& event) override;

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
  /// name of scoring plane collection we should use
  std::string coll_name_;
  /// name of pass for scoring plane collection we should use
  std::string pass_name_;
  /// list of plane ID numbers to select for (use all hits if empty)
  std::vector<int> select_planes_;
  /// list of primary vertices we will give over to the G4Event
  std::vector<G4PrimaryVertex*> primary_vertices_;
  /// enable logging
  enableLogging("FromScoringPlane")
};

}  // namespace generators
}  // namespace simcore

#endif  // SIMCORE_FROMSCORINGPLANE_H
