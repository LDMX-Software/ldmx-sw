/**
 * @file AnalysisUtils.cxx
 * @brief Collection of utility functions useful for analysis
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Tools/AnalysisUtils.h"

//-----------------//
//   C++  StdLib   //
//-----------------//
#include <string>
#include <tuple>

//----------//
//   ldmx   //
//----------//
#include "Framework/Exception/Exception.h"
#include "SimCore/Event/SimParticle.h"

//----------//
//   ROOT   //
//----------//
#include "TVector3.h"

namespace Analysis {

std::tuple<int, const ldmx::SimParticle *>
getRecoil(const std::map<int, ldmx::SimParticle> &particleMap) {
  // The recoil electron is "produced" in the dark brem geneartion
  for (const auto &[trackID, particle] : particleMap) {
    if (particle.getPdgID() == 11 and
        particle.getProcessType() ==
            ldmx::SimParticle::ProcessType::eDarkBrem) {
      return {trackID, &particle};
    }
  }
  // only get here if recoil electron was not "produced" by dark brem
  //   in this case (bkgd), we interpret the primary electron as also the recoil
  //   electron
  return {1, &(particleMap.at(1))};
}

// Search the recoil electrons daughters for a photon
// Check if the photon has daughters and if so, if they were produced by PN
//

bool doesParticleHavePNDaughters(
    const ldmx::SimParticle &gamma,
    const std::map<int, ldmx::SimParticle> &particleMap) {
  for (auto daughterID : gamma.getDaughters()) {
    if (particleMap.find(daughterID) != std::end(particleMap)) {
      const auto daughter{particleMap.at(daughterID)};
      const auto processType{daughter.getProcessType()};
      if (processType == ldmx::SimParticle::ProcessType::photonNuclear) {
        return true;
      } // Was it PN?
    }   // Was it in the map?
  }

  return false;
}

const ldmx::SimParticle *
getPNGamma(const std::map<int, ldmx::SimParticle> &particleMap,
           const ldmx::SimParticle *recoil, const float &energyThreshold) {
  auto recoilDaughters{recoil->getDaughters()};
  for (auto recoilDaughterID : recoilDaughters) {
    // Have we stored the recoil daughter?
    if (particleMap.find(recoilDaughterID) != std::end(particleMap)) {
      auto recoilDaughter{particleMap.at(recoilDaughterID)};
      // Is it a gamma?
      if (recoilDaughter.getPdgID() == 22) {
        // Does it have enough energy?
        if (recoilDaughter.getEnergy() >= energyThreshold) {
          // Are its daughters PN products?
          if (doesParticleHavePNDaughters(recoilDaughter, particleMap)) {
            return &particleMap.at(recoilDaughterID);
          }
        }
      }
    }
  }
  return nullptr;
}

} // namespace Analysis
