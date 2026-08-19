/**
 * @file PrimaryGenerator.h
 * @brief Header file for PrimaryGenerator
 */

#ifndef SIMCORE_PRIMARYGENERATOR_H
#define SIMCORE_PRIMARYGENERATOR_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <string>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4VPrimaryGenerator.hh"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/Factory.h"
#include "Framework/RunHeader.h"
#include "Framework/Event.h"

// Forward Declarations
class G4Event;
class G4PrimaryVertex;

namespace simcore {

/**
 * @class PrimaryGenerator
 * @brief Interface that defines a simulation primary generator.
 *
 * This class inherits from the Geant4 Primary Genertor template,
 * and is used as a common reference for all of the other PrimaryGenerators.
 */
class PrimaryGenerator : public G4VPrimaryGenerator {
 public:
  /**
   * Constructor.
   *
   * @param name Name given the to class instance.
   */
  PrimaryGenerator(const std::string& name,
                   const framework::config::Parameters& parameters);

  DECLARE_FACTORY_WITH_WAREHOUSE(PrimaryGenerator,
                                 std::shared_ptr<PrimaryGenerator>,
                                 const std::string&,
                                 const framework::config::Parameters&);

  /// Destructor
  virtual ~PrimaryGenerator() = default;

  /**
   * Prepare to generate a new primary vertex given the input event context
   *
   * This is helpful in the case where the primary vertices are being read
   * from the input LDMX Event file or if the primary generator just wants
   * to use some information from the EventHeader to help guide generation
   * of primary vertices.
   *
   * @param[in] event a const reference to the event that is about to be generated
   * It is const because objects should only be added to the event by the
   * Simulator (or ReSimulator) after an event is confirmed to not have been
   * aborted during the simulation.
   */
  virtual void PrepEvent(const framework::Event& event) {}

  /**
   * Generate a Primary Vertex
   *
   * This function must be defined by any other LDMX generators.
   */
  virtual void GeneratePrimaryVertex(G4Event*) = 0;

  /**
   * Record the configuration of the primary generator into the run header
   *
   * @note you must include the id number in each entry into the run header
   * just in case there are other generators
   */
  // NOLINTNEXTLINE
  virtual void RecordConfig(const std::string& id, ldmx::RunHeader& rh) = 0;

  std::string name() { return name_; }

  /**
   * Apply beam spot smearing to a primary vertex
   *
   * This method should be called by derived classes after generating a vertex
   * if beam spot smearing is enabled for this generator.
   *
   * @param primary_vertex The vertex to smear
   */
  void smearBeamspot(G4PrimaryVertex* primary_vertex);

  /**
   * Check if beam spot smearing is enabled for this generator
   *
   * @return true if smearing is enabled
   */
  bool useBeamspot() const { return use_beamspot_; }

 protected:
  /// Name of the PrimaryGenerator
  std::string name_{""};

  /// Flag denoting whether beam spot smearing is enabled for this generator
  bool use_beamspot_{false};

  /// Extent of the beamspot in x [mm]
  double beamspot_x_size_{0};

  /// Extent of the beamspot in y [mm]
  double beamspot_y_size_{0};

  /// Extent of the beamspot in z [mm]
  double beamspot_z_size_{0};
};  // PrimaryGenerator

}  // namespace simcore

/**
 * @macro DECLARE_GENERATOR
 *
 * Defines a builder for the declared class
 * and then registers the class as a generator
 * with the Factory
 */
#define DECLARE_GENERATOR(CLASS) \
  FACTORY_REGISTRATION(simcore::PrimaryGenerator, CLASS)

#endif  // SIMCORE_PRIMARYGENERATOR_H
