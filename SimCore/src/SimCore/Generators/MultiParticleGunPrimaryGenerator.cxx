/**
 * @file MultiParticleGunPrimaryGenerator.cxx
 * @brief Class for generating an event using multiple particles.
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Nhan Tran, FNAL
 */

#include "SimCore/Generators/MultiParticleGunPrimaryGenerator.h"

namespace simcore {
namespace generators {

MultiParticleGunPrimaryGenerator::MultiParticleGunPrimaryGenerator(
    const std::string& name, const framework::config::Parameters& parameters)
    : PrimaryGenerator(name, parameters), random_(new TRandom) {
  auto stl_vertex{parameters.get<std::vector<double> >("vertex")};
  auto stl_momentum{parameters.get<std::vector<double> >("momentum")};
  mpg_n_particles_ = parameters.get<int>("nParticles");
  mpgPdgID_ = parameters.get<int>("pdgID");
  mpg_enable_poisson_ = parameters.get<bool>("enablePoisson");

  if (stl_vertex.size() != 3 or stl_momentum.size() != 3 or mpg_n_particles_ <= 0) {
    EXCEPTION_RAISE("InvalideConfig",
                    "Parameters pass to '" + name_ + "' are not valid.");
  }

  mpg_vertex_ = G4ThreeVector(stl_vertex.at(0) * mm, stl_vertex.at(1) * mm,
                             stl_vertex.at(2) * mm);
  mpg_momentum_ = G4ThreeVector(stl_momentum.at(0) * MeV, stl_momentum.at(1) * MeV,
                               stl_momentum.at(2) * MeV);
}

MultiParticleGunPrimaryGenerator::~MultiParticleGunPrimaryGenerator() {
  delete random_;
}

void MultiParticleGunPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {
  int cur_mpg_pdgid = mpgPdgID_;
  G4ThreeVector cur_mpg_vertex = mpg_vertex_;
  G4ThreeVector cur_mpg_momentum = mpg_momentum_;

  // current number of vertices in the event!
  int cur_n_vertices = anEvent->GetNumberOfPrimaryVertex();

  double n_interactions_input = mpg_n_particles_;
  int n_interactions = n_interactions_input;
  if (mpgEnablePoisson_) {
    n_interactions = 0;
    while (n_interactions == 0) {  // keep generating a random poisson until > 0,
                                  // no point in generator 0 vertices...
      n_interactions = random_->Poisson(n_interactions_input);
    }
  }

  // make a for loop
  for (int i = 0; i < (n_interactions - cur_n_vertices); ++i) {
    G4PrimaryVertex* curvertex =
        new G4PrimaryVertex(cur_mpg_vertex, 0.);  // second input is t0
    // curvertex->SetPosition(0. * mm,0. * mm,-10. * mm);
    curvertex->SetWeight(1.);

    G4PrimaryParticle* primary =
        new G4PrimaryParticle(cur_mpg_pdgid, cur_mpg_momentum.x(),
                              cur_mpg_momentum.y(), cur_mpg_momentum.z());

    UserPrimaryParticleInformation* primary_info =
        new UserPrimaryParticleInformation();
    primary_info->setHepEvtStatus(1.);
    primary->SetUserInformation(primary_info);

    curvertex->SetPrimary(primary);
    anEvent->AddPrimaryVertex(curvertex);
  }
}

void MultiParticleGunPrimaryGenerator::recordConfig(const std::string& id,
                                                    ldmx::RunHeader& rh) {
  rh.setStringParameter(
      id + " Class", "simcore::generators::MultiParticleGunPrimaryGenerator");
  rh.setIntParameter(id + " Poisson Enabled", mpg_enable_poisson_);
  rh.setFloatParameter(id + " N Particles", mpg_n_particles_);
  rh.setIntParameter(id + " PDG ID", mpgPdgID_);
  rh.setFloatParameter(id + " Vertex X [mm]", mpg_vertex_.x());
  rh.setFloatParameter(id + " Vertex Y [mm]", mpg_vertex_.y());
  rh.setFloatParameter(id + " Vertex Z [mm]", mpg_vertex_.z());
  rh.setFloatParameter(id + " Momentum X [MeV]", mpg_momentum_.x());
  rh.setFloatParameter(id + " Momentum Y [MeV]", mpg_momentum_.y());
  rh.setFloatParameter(id + " Momentum Z [MeV]", mpg_momentum_.z());
}

}  // namespace generators
}  // namespace simcore

DECLARE_GENERATOR(simcore::generators::MultiParticleGunPrimaryGenerator)
