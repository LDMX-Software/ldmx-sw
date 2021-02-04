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

// Forward Declarations
class G4Event;

namespace simcore {

// Forward declarations
class PrimaryGenerator;

typedef PrimaryGenerator* PrimaryGeneratorBuilder(
    const std::string& name, framework::config::Parameters& parameters);

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
                   framework::config::Parameters& parameters);

  /// Destructor
  virtual ~PrimaryGenerator();

  /**
   * Method used to register a user action with the manager.
   *
   * @param className Name of the class instance
   * @param builder The builder used to create and instance of this class.
   */
  static void declare(const std::string& className,
                      PrimaryGeneratorBuilder* builder);

  /**
   * Generate a Primary Vertex
   *
   * This function must be defined by any other LDMX generators.
   */
  virtual void GeneratePrimaryVertex(G4Event*) = 0;

 protected:
  /// Name of the PrimaryGenerator
  std::string name_{""};

  /// The set of parameters used to configure this class
  framework::config::Parameters parameters_;

};  // PrimaryGenerator

}  // namespace simcore

/**
 * @macro DECLARE_GENERATOR
 *
 * Defines a builder for the declared class
 * and then registers the class as a generator
 * with the PrimaryGeneratorManager
 */
#define DECLARE_GENERATOR(NS, CLASS)                                        \
  simcore::PrimaryGenerator* CLASS##Builder(                                \
      const std::string& name, framework::config::Parameters& parameters) { \
    return new NS::CLASS(name, parameters);                                 \
  }                                                                         \
  __attribute((constructor(305))) static void CLASS##Declare() {            \
    simcore::PrimaryGenerator::declare(                                     \
        std::string(#NS) + "::" + std::string(#CLASS), &CLASS##Builder);    \
  }

#endif  // SIMCORE_PRIMARYGENERATOR_H
