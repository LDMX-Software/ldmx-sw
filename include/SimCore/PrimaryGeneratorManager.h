/**
 * @file PrimaryGeneratorManager.h
 * @brief Class that manages the generators used to fire particles.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_PRIMARYGENERATORMANAGER_H
#define SIMCORE_PRIMARYGENERATORMANAGER_H

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "SimCore/PrimaryGenerator.h"

namespace simcore {

/**
 * @class PrimaryGeneratorManager
 * @brief Class that manages the generators used to fire particles.
 */
class PrimaryGeneratorManager {
 public:
  /// @return the global PrimaryGeneratorManager instance
  static PrimaryGeneratorManager& getInstance();

  /**
   * Get the collection of all enabled generators
   */
  std::vector<PrimaryGenerator*> getGenerators() const { return generators_; };

  /**
   * Attach a new generator to the list of generators
   */
  void registerGenerator(const std::string& className,
                         PrimaryGeneratorBuilder* builder);

  /**
   * Create a new generate and attach it to the list of generators
   */
  void createGenerator(const std::string& className,
                       const std::string& instanceName, framework::config::Parameters& parameters);

 private:
  /// PrimaryGeneratorManager instance
  static PrimaryGeneratorManager instance_;

  /// Constructor - private to prevent initialization
  PrimaryGeneratorManager() {}

  /**
   * @struct GeneratorInfo
   * @brief Holds necessary information to create a generator
   */
  struct GeneratorInfo {
    /// Name of the Class
    std::string className_;

    /// Class builder
    PrimaryGeneratorBuilder* builder_;
  };

  /// A map of all register generators
  std::map<std::string, GeneratorInfo> generatorMap_;

  /// Cointainer for all generators to be used by the simulation
  std::vector<PrimaryGenerator*> generators_;

};  // PrimaryGeneratorManager

}  // namespace simcore

#endif  // SIMCORE_PRIMARYGENERATORMANAGER_H
