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
#include "Framework/RunHeader.h"
#include "SimCore/Factory.h"

// Forward Declarations
class G4Event;

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

  /// Factory for primary generators
  using Factory =
      ::simcore::Factory<PrimaryGenerator, std::shared_ptr<PrimaryGenerator>,
                         const std::string&,
                         const framework::config::Parameters&>;

  /// Destructor
  virtual ~PrimaryGenerator() = default;

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
  virtual void RecordConfig(const std::string& id, ldmx::RunHeader& rh) = 0;

 protected:
  /// Name of the PrimaryGenerator
  std::string name_{""};
};  // PrimaryGenerator

}  // namespace simcore

/**
 * @macro DECLARE_GENERATOR
 *
 * Defines a builder for the declared class
 * and then registers the class as a generator
 * with the Factory
 */
#define DECLARE_GENERATOR(CLASS)                                         \
  namespace {                                                            \
  auto v = ::simcore::PrimaryGenerator::Factory::get().declare<CLASS>(); \
  }

#endif  // SIMCORE_PRIMARYGENERATOR_H
