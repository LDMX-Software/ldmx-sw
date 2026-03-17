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
#include "Math/Vector3D.h"

namespace analysis {

std::tuple<int, const ldmx::SimParticle *> getRecoil(
    const std::map<int, ldmx::SimParticle> &particleMap) {
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
  if (particleMap.find(1) != particleMap.end()) {
    return {1, &(particleMap.at(1))};
  } else {
    // need to account for overlay tracks, which replace track ID
    // 1 with 134217729 in the main sample and with 150994945 in the
    // pileup
    // this code right now is decidedly NOT future proof and only handles a
    // single encoding version
    return {134217729, &(particleMap.at(134217729))};
  }
}

// Search the recoil electrons daughters for a photon
// Check if the photon has daughters and if so, if they were produced by PN
//

bool doesParticleHavePNDaughters(
    const ldmx::SimParticle &gamma,
    const std::map<int, ldmx::SimParticle> &particleMap) {
  for (auto daughter_id : gamma.getDaughters()) {
    if (particleMap.find(daughter_id) != std::end(particleMap)) {
      const auto daughter{particleMap.at(daughter_id)};
      const auto process_type{daughter.getProcessType()};
      if (process_type == ldmx::SimParticle::ProcessType::photonNuclear) {
        return true;
      }  // Was it PN?
    }  // Was it in the map?
  }

  return false;
}

const ldmx::SimParticle *getPNGamma(
    const std::map<int, ldmx::SimParticle> &particleMap,
    const ldmx::SimParticle *recoil, const float &energyThreshold) {
  auto recoil_daughters{recoil->getDaughters()};
  for (auto recoil_daughter_id : recoil_daughters) {
    // Have we stored the recoil daughter?
    if (particleMap.find(recoil_daughter_id) != std::end(particleMap)) {
      auto recoil_daughter{particleMap.at(recoil_daughter_id)};
      // Is it a gamma?
      if (recoil_daughter.getPdgID() == 22) {
        // Does it have enough energy?
        if (recoil_daughter.getEnergy() >= energyThreshold) {
          // Are its daughters PN products?
          if (doesParticleHavePNDaughters(recoil_daughter, particleMap)) {
            return &particleMap.at(recoil_daughter_id);
          }
        }
      }
    }
  }
  return nullptr;
}

}  // namespace analysis
