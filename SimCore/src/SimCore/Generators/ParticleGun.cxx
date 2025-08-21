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
  verbosity_ = parameters.get<int>("verbosity");

  auto particle_table{G4ParticleTable::GetParticleTable()};

  auto particle{parameters.get<std::string>("particle")};
  if (auto particle_def{particle_table->FindParticle(particle)};
      particle_def != 0) {
    if (verbosity_ > 1) {
      std::cout << "[ ParticleGun ] : Firing particle of type " << particle
                << std::endl;
    }
    the_gun_.SetParticleDefinition(particle_def);
  }

  auto energy{parameters.get<double>("energy")};
  if (verbosity_ > 1) {
    std::cout << "[ ParticleGun ] : Setting energy to " << energy * GeV
              << std::endl;
  }
  the_gun_.SetParticleEnergy(energy * GeV);

  auto position{parameters.get<std::vector<double> >("position")};
  if (!position.empty()) {
    G4ThreeVector p_vec(position[0] * mm, position[1] * mm, position[2] * mm);
    if (verbosity_ > 1) {
      std::cout << "[ ParticleGun ] : position " << p_vec << std::endl;
    }
    the_gun_.SetParticlePosition(p_vec);
  }

  auto time{parameters.get<double>("time")};
  if (time < 0) time = 0.0;
  if (verbosity_ > 1) {
    std::cout << "[ ParticleGun ] : Setting particle time  to " << time
              << std::endl;
  }
  the_gun_.SetParticleTime(time * ns);

  auto direction{parameters.get<std::vector<double> >("direction")};
  if (!direction.empty()) {
    G4ThreeVector d_vec(direction[0], direction[1], direction[2]);
    if (verbosity_ > 1) {
      std::cout << "[ ParticleGun ] : direction " << d_vec.unit() << std::endl;
    }
    the_gun_.SetParticleMomentumDirection(d_vec);
  }
}

void ParticleGun::GeneratePrimaryVertex(G4Event* event) {
  // Call G4 class method to generate primaries.
  the_gun_.GeneratePrimaryVertex(event);
}

void ParticleGun::recordConfig(const std::string& id, ldmx::RunHeader& rh) {
  rh.setStringParameter(id + " Class", "simcore::generators::ParticleGun");
  rh.setFloatParameter(id + " Time [ns]", the_gun_.GetParticleTime());
  rh.setFloatParameter(id + " Energy [GeV]", the_gun_.GetParticleEnergy() / GeV);
  rh.setStringParameter(id + " Particle",
                        the_gun_.GetParticleDefinition()->GetParticleName());
  rh.setFloatParameter(id + " X [mm]", the_gun_.GetParticlePosition().x());
  rh.setFloatParameter(id + " Y [mm]", the_gun_.GetParticlePosition().y());
  rh.setFloatParameter(id + " Z [mm]", the_gun_.GetParticlePosition().z());
  rh.setFloatParameter(id + " Direction X",
                       the_gun_.GetParticleMomentumDirection().x());
  rh.setFloatParameter(id + " Direction Y",
                       the_gun_.GetParticleMomentumDirection().y());
  rh.setFloatParameter(id + " Direction Z",
                       the_gun_.GetParticleMomentumDirection().z());
}

}  // namespace generators
}  // namespace simcore

DECLARE_GENERATOR(simcore::generators::ParticleGun)
