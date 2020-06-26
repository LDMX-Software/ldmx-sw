/**
 * @file GammaToMuPairXsecBiasingPlugin.cxx
 * @brief Geant4 Biasing Operator used to bias the occurence of muon pair 
 *        conversions by modifying the cross-section.
 * @author Omar Moreno
 *         SLAC National Accelerator Laboratory
 */

#include "SimCore/GammaToMuPairXsecBiasingOperator.h"

namespace ldmx { 

    const std::string GammaToMuPairXsecBiasingOperator::GAMMATOMUPAIR_PROCESS = "GammaToMuPair";

    GammaToMuPairXsecBiasingOperator::GammaToMuPairXsecBiasingOperator(std::string name) : 
        PhotoNuclearXsecBiasingOperator(name) { 
    }

    GammaToMuPairXsecBiasingOperator::~GammaToMuPairXsecBiasingOperator() { 
    }
}
