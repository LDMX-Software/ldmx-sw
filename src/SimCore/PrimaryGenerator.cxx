/**
 * @file PrimaryGenerator.cxx
 * @brief Implementation file for PrimaryGenerator
 *
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/PrimaryGenerator.h"

namespace simcore {

PrimaryGenerator::PrimaryGenerator(const std::string& name,
                                   const framework::config::Parameters& parameters) :
  name_(name) {}


}  // namespace simcore
