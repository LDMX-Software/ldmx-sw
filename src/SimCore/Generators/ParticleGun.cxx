/**
 * @file ParticleGun.cxx
 * @brief Extension of G4ParticleGun.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/Generators/ParticleGun.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <memory>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"

namespace simcore {
namespace generators {

ParticleGun::ParticleGun(const std::string& name, 
    const framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters) {
  verbosity_ = parameters.getParameter<int>("verbosity");

  auto particleTable{G4ParticleTable::GetParticleTable()};

  auto particle{parameters.getParameter<std::string>("particle")};
  if (auto particleDef{particleTable->FindParticle(particle)};
      particleDef != 0) {
    if (verbosity_ > 1) {
      std::cout << "[ ParticleGun ] : Firing particle of type " << particle
                << std::endl;
    }
    theGun_.SetParticleDefinition(particleDef);
  }

  auto energy{parameters.getParameter<double>("energy")};
  if (verbosity_ > 1) {
    std::cout << "[ ParticleGun ] : Setting energy to " << energy * GeV
              << std::endl;
  }
  theGun_.SetParticleEnergy(energy * GeV);

  auto position{parameters.getParameter<std::vector<double> >("position")};
  if (!position.empty()) {
    G4ThreeVector pVec(position[0] * mm, position[1] * mm, position[2] * mm);
    if (verbosity_ > 1) {
      std::cout << "[ ParticleGun ] : position " << pVec << std::endl;
    }
    theGun_.SetParticlePosition(pVec);
  }

  auto time{parameters.getParameter<double>("time")};
  if (time < 0) time = 0.0;
  if (verbosity_ > 1) {
    std::cout << "[ ParticleGun ] : Setting particle time  to " << time
              << std::endl;
  }
  theGun_.SetParticleTime(time * ns);

  auto direction{parameters.getParameter<std::vector<double> >("direction")};
  if (!direction.empty()) {
    G4ThreeVector dVec(direction[0], direction[1], direction[2]);
    if (verbosity_ > 1) {
      std::cout << "[ ParticleGun ] : direction " << dVec.unit() << std::endl;
    }
    theGun_.SetParticleMomentumDirection(dVec);
  }
}

ParticleGun::~ParticleGun() {}

void ParticleGun::GeneratePrimaryVertex(G4Event* event) {
  // Call G4 class method to generate primaries.
  theGun_.GeneratePrimaryVertex(event);
}

}  // namespace generators
}  // namespace simcore

DECLARE_GENERATOR(simcore::generators, ParticleGun)
