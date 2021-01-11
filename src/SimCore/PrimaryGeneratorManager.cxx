/**
 * @file PrimaryGeneratorManager.cxx
 * @brief Class that manages the generators used to fire particles.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/PrimaryGeneratorManager.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <dlfcn.h>
#include <algorithm>
#include <string>
#include <vector>

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Exception/Exception.h"

simcore::PrimaryGeneratorManager simcore::PrimaryGeneratorManager::instance_
    __attribute__((init_priority(300)));

namespace simcore {

PrimaryGeneratorManager& PrimaryGeneratorManager::getInstance() {
  return instance_;
}

void PrimaryGeneratorManager::registerGenerator(
    const std::string& className, PrimaryGeneratorBuilder* builder) {
  GeneratorInfo info;
  info.className_ = className;
  info.builder_ = builder;

  generatorMap_[className] = info;
}

void PrimaryGeneratorManager::createGenerator(const std::string& className,
                                              const std::string& instanceName,
                                              framework::config::Parameters& parameters) {
  auto it{generatorMap_.find(className)};
  if (it == generatorMap_.end()) {
    EXCEPTION_RAISE("CreateGenerator",
                    "Failed to create generator '" + className + "'.");
  }

  auto generator{it->second.builder_(instanceName, parameters)};

  // now that the generator is built --> put it on active list
  generators_.push_back(generator);
}

}  // namespace simcore
