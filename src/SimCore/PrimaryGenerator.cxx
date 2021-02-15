/**
 * @file PrimaryGenerator.cxx
 * @brief Implementation file for PrimaryGenerator
 *
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/PrimaryGenerator.h"

#include "SimCore/PrimaryGeneratorManager.h"

namespace simcore {

PrimaryGenerator::PrimaryGenerator(const std::string& name,
                                   framework::config::Parameters& parameters) {
  name_ = name;
  parameters_ = parameters;
}

PrimaryGenerator::~PrimaryGenerator() {}

void PrimaryGenerator::declare(const std::string& className,
                               PrimaryGeneratorBuilder* builder) {
  PrimaryGeneratorManager::getInstance().registerGenerator(className, builder);
}
}  // namespace simcore
