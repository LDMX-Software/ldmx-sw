#include "SimCore/SDs/HcalSD.h"

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"

// STL
#include <iostream>

// Geant4
#include "G4Box.hh"
#include "G4ParticleTypes.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

namespace simcore {

const std::string HcalSD::COLLECTION_NAME = "HcalSimHits";

HcalSD::HcalSD(const std::string& name, simcore::ConditionsInterface& ci,
               const framework::config::Parameters& p)
    : SensitiveDetector(name, ci, p), birksc1_(1.29e-2), birksc2_(9.59e-6) {
  gdml_identifiers_ = {p.get<std::vector<std::string>>("gdml_identifiers")};
}

ldmx::HcalID HcalSD::decodeCopyNumber(const std::uint32_t copyNumber,
                                      const G4ThreeVector& localPosition,
                                      const G4Box* scint) {
  const unsigned int version{copyNumber / 0x01000000};
  if (version != 0) {
    using Index = ldmx::PackedIndex<256, 256, 256>;
    return ldmx::HcalID{Index(copyNumber).field2(), Index(copyNumber).field1(),
                        Index(copyNumber).field0()};
  }
  const auto& geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
  unsigned int strip_id = 0;
  const unsigned int section = copyNumber / 1000;
  const unsigned int layer = copyNumber % 1000;

  // 5cm wide bars are HARD-CODED
  if (section == ldmx::HcalID::BACK) {
    if (geometry.backLayerIsHorizontal(layer)) {
      strip_id = int((localPosition.y() + scint->GetYHalfLength()) / 50.0);
    } else {
      strip_id = int((localPosition.x() + scint->GetXHalfLength()) / 50.0);
    }
  } else {
    strip_id = int((localPosition.z() + scint->GetZHalfLength()) / 50.0);
  }
  return ldmx::HcalID{section, layer, strip_id};
}

G4bool HcalSD::ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) {
  // Get the edep from the step.
  G4double edep = aStep->GetTotalEnergyDeposit();

  // Skip steps with no energy dep which come from non-Geantino particles.
  if (edep == 0.0 and not isGeantino(aStep)) {
    if (verboseLevel > 2) {
      std::cout << "CalorimeterSD skipping step with zero edep." << std::endl
                << std::endl;
    }
    return false;
  }

  //---------------------------------------------------------------------------------------------------
  //                Birks' Law
  //                ===========
  //
  //                      In the case of Scintillator as active medium, we can
  //              describe the quenching effects with the Birks' law,
  //              using the expression and the coefficients taken from
  //              the paper NIM 80 (1970) 239-244 for the organic
  //              scintillator NE-102:
  //                                     S*dE/dr
  //                    dL/dr = -----------------------------------
  //                               1 + C1*(dE/dr)
  //              with:
  //                    S=1
  //                    C1 = 1.29 x 10^-2  g*cm^-2*MeV^-1
  //                    C2 = 9.59 x 10^-6  g^2*cm^-4*MeV^-2
  //              These are the same values used by ATLAS TileCal
  //              and CMS HCAL (and also the default in Geant3).
  //              You can try different values for these parameters,
  //              to have an idea on the uncertainties due to them,
  //              by uncommenting one of the lines below.
  //              To get the "dE/dr" that appears in the formula,
  //              which has the dimensions
  //                    [ dE/dr ] = MeV * cm^2 / g
  //              we have to divide the energy deposit in MeV by the
  //              product of the step length (in cm) and the density
  //              of the scintillator:

  G4double birks_factor(1.0);
  G4double step_length = aStep->GetStepLength() / CLHEP::cm;
  // Do not apply Birks for gamma deposits!
  // Check, cut if necessary.
  if (step_length > 1.0e-6) {
    G4double rho = aStep->GetPreStepPoint()->GetMaterial()->GetDensity() /
                   (CLHEP::g / CLHEP::cm3);
    G4double dedx = edep / (rho * step_length);  //[MeV*cm^2/g]
    birks_factor = 1.0 / (1.0 + birksc1_ * dedx + birksc2_ * dedx * dedx);
    if (aStep->GetTrack()->GetDefinition() == G4Gamma::GammaDefinition())
      birks_factor = 1.0;
    if (aStep->GetTrack()->GetDefinition() == G4Neutron::NeutronDefinition())
      birks_factor = 1.0;
  }

  // update edep to include birksFactor
  edep *= birks_factor;

  // Create a new cal hit.
  ldmx::SimCalorimeterHit& hit{hits_.emplace_back()};

  // Get the scintillator solid box
  G4Box* scint = nullptr;

  if (aStep) {
    const auto* pre_step_point = aStep->GetPreStepPoint();
    if (pre_step_point) {
      const auto& touchable_handle = pre_step_point->GetTouchableHandle();
      if (touchable_handle) {
        const auto* volume = touchable_handle->GetVolume();

        if (volume) {
          const auto* logical_volume = volume->GetLogicalVolume();
          if (logical_volume) {
            auto* solid = logical_volume->GetSolid();
            if (solid) {
              scint = static_cast<G4Box*>(solid);
            }
          }
        }
      }
    }
  }

  // Set the step mid-point as the hit position.
  G4StepPoint* pre_point = aStep->GetPreStepPoint();
  G4StepPoint* post_point = aStep->GetPostStepPoint();
  // A Geant4 "touchable" is a way to uniquely identify a particular volume,
  // short for touchable detector element. See the detector definition and
  // response section of the Geant4 application developers manual for details.
  //
  // The TouchableHandle is just a reference counted pointer to a
  // G4TouchableHistory object, which is a concrete implementation of a
  // G4Touchable interface.
  //
  auto touchable_history{pre_point->GetTouchableHandle()->GetHistory()};
  // Affine transform for converting between local and global coordinates
  auto top_transform{touchable_history->GetTopTransform()};
  G4ThreeVector position =
      0.5 * (pre_point->GetPosition() + post_point->GetPosition());
  G4ThreeVector local_position = top_transform.TransformPoint(position);
  hit.setPosition(position[0], position[1], position[2]);

  // Create the ID for the hit. Note 2 here corresponds to the "depth" of the
  // geometry tree. If this changes in the GDML, this would have to be updated
  // here. Currently, 0 corresponds to the world volume, 1 corresponds to the
  // Hcal, and 2 to the bars/absorbers
  int copy_num = touchable_history->GetVolume(2)->GetCopyNo();
  ldmx::HcalID id = decodeCopyNumber(copy_num, local_position, scint);
  hit.setID(id.raw());

  // add one contributor for this hit with
  //  ID of ancestor incident on Cal-Region
  //  ID of this track
  //  PDG of this track
  //  EDEP (including birks factor)
  //  time of this hit
  const G4Track* track = aStep->GetTrack();
  int track_id = track->GetTrackID();
  hit.addContrib(getTrackMap().findIncident(track_id), track_id,
                 track->GetParticleDefinition()->GetPDGEncoding(), edep,
                 track->GetGlobalTime());
  //
  // Pre/post step details for scintillator response simulation

  // Convert back to mm
  hit.setPathLength(step_length * CLHEP::cm / CLHEP::mm);
  hit.setVelocity(track->GetVelocity());
  const auto& geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
  // Convert pre/post step position from global coordinates to coordinates
  // within the scintillator bar
  const auto local_pre_step_point{
      top_transform.TransformPoint(pre_point->GetPosition())};
  const auto local_post_step_point{
      top_transform.TransformPoint(post_point->GetPosition())};

  // And rotate them to a local coordinate system for the bar that always has
  // the same x/y/z definitions (see HcalGeometry for details)
  auto local_pre_position_rotated{geometry.rotateGlobalToLocalBarPosition(
      {local_pre_step_point[0], local_pre_step_point[1], local_pre_step_point[2]}, id)};

  auto local_post_position_rotated{geometry.rotateGlobalToLocalBarPosition(
      {local_post_step_point[0], local_post_step_point[1], local_post_step_point[2]},
      id)};
  hit.setPreStepPosition(local_pre_position_rotated[0], local_pre_position_rotated[1],
                         local_pre_position_rotated[2]);
  hit.setPostStepPosition(local_post_position_rotated[0],
                          local_post_position_rotated[1],
                          local_post_position_rotated[2]);
  hit.setPreStepTime(pre_point->GetGlobalTime());
  hit.setPostStepTime(post_point->GetGlobalTime());

  ldmx_log(trace) << hit;

  return true;
}

}  // namespace simcore

DECLARE_SENSITIVEDETECTOR(simcore::HcalSD)
