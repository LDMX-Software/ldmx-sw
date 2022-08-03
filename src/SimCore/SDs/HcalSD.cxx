#include "SimCore/SDs/HcalSD.h"

/*~~~~~~~~~~~~~~*/
/*   DetDescr   */
/*~~~~~~~~~~~~~~*/
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
    : SensitiveDetector(name, ci, p), birksc1_(1.29e-2), birksc2_(9.59e-6) {}

HcalSD::~HcalSD() {}

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
  G4ThreeVector position =
      0.5 * (prePoint->GetPosition() + postPoint->GetPosition());
  G4ThreeVector localPosition = aStep->GetPreStepPoint()
                                    ->GetTouchableHandle()
                                    ->GetHistory()
                                    ->GetTopTransform()
                                    .TransformPoint(position);
  hit.setPosition(position[0], position[1], position[2]);

  // Create the ID for the hit.
  int copyNum = aStep->GetPreStepPoint()
                    ->GetTouchableHandle()
                    ->GetHistory()
                    ->GetVolume(2)
                    ->GetCopyNo();
  int section = copyNum / 1000;
  int layer = copyNum % 1000;

  // stripID: back Hcal, segmented along y direction for now every 10 cm --
  // alternate x-y in the future?
  //         left/right side hcal: segmented along x direction every 10 cm
  //         top/bottom side hcal: segmented along y direction every 10 cm

  int stripID = -1;
  // Odd layers have bars horizontal
  // Even layers have bars vertical
  // 5cm wide bars are HARD-CODED
  if (section == ldmx::HcalID::BACK && layer % 2 == 1)
    stripID = int((localPosition.y() + scint->GetYHalfLength()) / 50.0);
  else if (section == ldmx::HcalID::BACK && layer % 2 == 0)
    stripID = int((localPosition.x() + scint->GetXHalfLength()) / 50.0);
  else
    stripID = int((localPosition.z() + scint->GetZHalfLength()) / 50.0);

  // std::cout << "---" << std::endl;
  // std::cout << "GetXHalfLength = " << scint->GetXHalfLength() << "\t
  // GetYHalfLength = " << scint->GetYHalfLength() << "\t GetZHalfLength = " <<
  // scint->GetZHalfLength() << std::endl; std::cout << "xpos = " <<
  // localPosition.x() << "\t ypos = " << localPosition.y() << "\t zpos = " <<
  // localPosition.z() << std::endl; std::cout << "xpos_g = " << position.x() <<
  // "\t ypos_g = " << position.y() << "\t zpos_g = " << position.z() <<
  // std::endl; std::cout << "Layer = " << layer << "\t section = " << section
  // << "\t strip = " << stripID << std::endl;

  ldmx::HcalID id(section, layer, stripID);
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
                 track->GetParticleDefinition()->GetPDGEncoding(),
                 edep, track->GetGlobalTime());

  // do we want to set the hit coordinate in the middle of the absorber?
  // G4ThreeVector volumePosition =
  // aStep->GetPreStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().Inverse().TransformPoint(G4ThreeVector());
  // if (section==ldmx::HcalID::BACK) hit->setPosition(position[0], position[1],
  // volumePosition.z()); elseif (section==ldmx::HcalID::TOP ||
  // section==ldmx::HcalID::BOTTOM) hit->setPosition(position[0], volumePosition.y(),
  // position[2]); elseif (section==ldmx::HcalID::LEFT || section==ldmx::HcalID::RIGHT)
  // hit->setPosition(volumePosition.x(),position[1] , position[2]);

  if (this->verboseLevel > 2) { hit.Print(); }

  return true;
}

}  // namespace simcore

DECLARE_SENSITIVEDETECTOR(simcore, HcalSD)
