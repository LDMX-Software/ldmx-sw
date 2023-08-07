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
  gdmlIdentifiers_ = {
      p.getParameter<std::vector<std::string>>("gdml_identifiers")};
}

HcalSD::~HcalSD() {}
ldmx::HcalID HcalSD::decodeCopyNumber(const std::uint32_t copyNumber,
                                      const G4ThreeVector& localPosition,
                                      const G4Box* scint) {
  const unsigned int version{copyNumber / 0x01000000};
  if (version != 0) {
    typedef ldmx::PackedIndex<256, 256, 256> Index;
    return ldmx::HcalID{Index(copyNumber).field2(), Index(copyNumber).field1(),
                        Index(copyNumber).field0()};
  } else {
    const auto& geometry = getCondition<ldmx::HcalGeometry>(
        ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
    unsigned int stripID = 0;
    const unsigned int section = copyNumber / 1000;
    const unsigned int layer = copyNumber % 1000;

    // 5cm wide bars are HARD-CODED
    if (section == ldmx::HcalID::BACK) {
      if (geometry.backLayerIsHorizontal(layer)) {
        stripID = int((localPosition.y() + scint->GetYHalfLength()) / 50.0);
      } else {
        stripID = int((localPosition.x() + scint->GetXHalfLength()) / 50.0);
      }
    } else {
      stripID = int((localPosition.z() + scint->GetZHalfLength()) / 50.0);
    }
    return ldmx::HcalID{section, layer, stripID};
  }
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

  G4double birksFactor(1.0);
  G4double stepLength = aStep->GetStepLength() / CLHEP::cm;
  // Do not apply Birks for gamma deposits!
  if (stepLength > 1.0e-6)  // Check, cut if necessary.
  {
    G4double rho = aStep->GetPreStepPoint()->GetMaterial()->GetDensity() /
                   (CLHEP::g / CLHEP::cm3);
    G4double dedx = edep / (rho * stepLength);  //[MeV*cm^2/g]
    birksFactor = 1.0 / (1.0 + birksc1_ * dedx + birksc2_ * dedx * dedx);
    if (aStep->GetTrack()->GetDefinition() == G4Gamma::GammaDefinition())
      birksFactor = 1.0;
    if (aStep->GetTrack()->GetDefinition() == G4Neutron::NeutronDefinition())
      birksFactor = 1.0;
  }

  // update edep to include birksFactor
  edep *= birksFactor;

  // Create a new cal hit.
  ldmx::SimCalorimeterHit& hit{hits_.emplace_back()};

  // Get the scintillator solid box
  G4Box* scint = static_cast<G4Box*>(aStep->GetPreStepPoint()
                                         ->GetTouchableHandle()
                                         ->GetVolume()
                                         ->GetLogicalVolume()
                                         ->GetSolid());

  // Set the step mid-point as the hit position.
  G4StepPoint* prePoint = aStep->GetPreStepPoint();
  G4StepPoint* postPoint = aStep->GetPostStepPoint();
  // A Geant4 "touchable" is a way to uniquely identify a particular volume,
  // short for touchable detector element. See the detector definition and
  // response section of the Geant4 application developers manual for details.
  //
  // The TouchableHandle is just a reference counted pointer to a
  // G4TouchableHistory object, which is a concrete implementation of a
  // G4Touchable interface.
  //
  auto touchableHistory{prePoint->GetTouchableHandle()->GetHistory()};
  // Affine transform for converting between local and global coordinates
  auto topTransform{touchableHistory->GetTopTransform()};
  G4ThreeVector position =
      0.5 * (prePoint->GetPosition() + postPoint->GetPosition());
  G4ThreeVector localPosition = topTransform.TransformPoint(position);
  hit.setPosition(position[0], position[1], position[2]);

  // Create the ID for the hit. Note 2 here corresponds to the "depth" of the
  // geometry tree. If this changes in the GDML, this would have to be updated
  // here. Currently, 0 corresponds to the world volume, 1 corresponds to the
  // Hcal, and 2 to the bars/absorbers
  int copyNum = touchableHistory->GetVolume(2)->GetCopyNo();
  ldmx::HcalID id = decodeCopyNumber(copyNum, localPosition, scint);
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
  hit.setPathLength(stepLength * CLHEP::cm / CLHEP::mm);
  hit.setVelocity(track->GetVelocity());
  const auto& geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
  // Convert pre/post step position from global coordinates to coordinates
  // within the scintillator bar
  const auto localPreStepPoint{
      topTransform.TransformPoint(prePoint->GetPosition())};
  const auto localPostStepPoint{
      topTransform.TransformPoint(postPoint->GetPosition())};

  // And rotate them to a local coordinate system for the bar that always has
  // the same x/y/z definitions (see HcalGeometry for details)
  auto localPrePositionRotated{geometry.rotateGlobalToLocalBarPosition(
      {localPreStepPoint[0], localPreStepPoint[1], localPreStepPoint[2]}, id)};

  auto localPostPositionRotated{geometry.rotateGlobalToLocalBarPosition(
      {localPostStepPoint[0], localPostStepPoint[1], localPostStepPoint[2]},
      id)};
  hit.setPreStepPosition(localPrePositionRotated[0], localPrePositionRotated[1],
                         localPrePositionRotated[2]);
  hit.setPostStepPosition(localPostPositionRotated[0],
                          localPostPositionRotated[1],
                          localPostPositionRotated[2]);
  hit.setPreStepTime(prePoint->GetGlobalTime());
  hit.setPostStepTime(postPoint->GetGlobalTime());

  if (this->verboseLevel > 2) {
    hit.Print();
  }

  return true;
}

}  // namespace simcore

DECLARE_SENSITIVEDETECTOR(simcore::HcalSD)
