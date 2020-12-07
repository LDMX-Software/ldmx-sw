/**
 * @file PrimaryGenerator.cxx
 * @brief Implementation file for PrimaryGenerator
 *
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/PrimaryGenerator.h"

#include "SimCore/PluginFactory.h"

namespace ldmx { 

    PrimaryGenerator::PrimaryGenerator(const std::string& name, Parameters& parameters) { 
        name_ = name; 
        parameters_ = parameters; 
    }

    PrimaryGenerator::~PrimaryGenerator() {} 

    void PrimaryGenerator::declare(const std::string& className, PrimaryGeneratorBuilder* builder) {
      simcore::PluginFactory::getInstance().registerGenerator(className, builder);      
    }
}
