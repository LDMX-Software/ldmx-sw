/**
 * @file PrimaryGenerator.cxx
 * @brief Implementation file for PrimaryGenerator
 *
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/Generators/PrimaryGenerator.h"

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "Randomize.hh"

namespace simcore {

PrimaryGenerator::PrimaryGenerator(
    const std::string& name, const framework::config::Parameters& parameters)
    : name_(name) {
  // Check if beam spot smearing is configured for this generator
  auto beam_spot{parameters.get<std::vector<double>>("beam_spot_smear", {})};
  if (!beam_spot.empty()) {
    use_beamspot_ = true;
    beamspot_x_size_ = beam_spot[0];
    beamspot_y_size_ = beam_spot[1];
    beamspot_z_size_ = beam_spot.size() > 2 ? beam_spot[2] : 0.;
  }
}

void PrimaryGenerator::smearBeamspot(G4PrimaryVertex* primary_vertex) {
  if (!use_beamspot_ || !primary_vertex) return;

  double x0_i = primary_vertex->GetX0();
  double y0_i = primary_vertex->GetY0();
  double z0_i = primary_vertex->GetZ0();

  /*
   * G4UniformRand returns a number in [0,1]
   *  - we shift this range so that it is [-0.5,0.5]
   *  - multiply by the width to get [-0.5*size,0.5*size]
   *  - add the initial point (in case its off center) to get
   *    [init-0.5*size, init+0.5*size]
   */
  double x0_f = beamspot_x_size_ * (G4UniformRand() - 0.5) + x0_i;
  double y0_f = beamspot_y_size_ * (G4UniformRand() - 0.5) + y0_i;
  double z0_f = beamspot_z_size_ * (G4UniformRand() - 0.5) + z0_i;
  primary_vertex->SetPosition(x0_f, y0_f, z0_f);
}

DEFINE_FACTORY(PrimaryGenerator);

}  // namespace simcore
