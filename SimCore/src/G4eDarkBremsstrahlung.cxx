/**
 * @file G4eDarkBremsstrahlung.cxx
 * @brief Class providing the Dark Bremsstrahlung process class.
 * @author Michael Revering, University of Minnesota
 */

#include "SimCore/G4eDarkBremsstrahlung.h"
#include "SimCore/G4APrime.h"
#include "SimCore/G4eDarkBremsstrahlungModel.h"

#include "G4DynamicParticle.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"

G4eDarkBremsstrahlung::G4eDarkBremsstrahlung(const G4String& name):
    G4VEnergyLossProcess(name),
    isInitialised(false) {  
    G4int subtype = 63;   
    SetProcessSubType(subtype);
    SetSecondaryParticle(G4APrime::APrime());
    SetIonisation(false);
}

G4eDarkBremsstrahlung::~G4eDarkBremsstrahlung() {
    if ( theModel_ ) delete theModel_;
}

G4bool G4eDarkBremsstrahlung::IsApplicable(const G4ParticleDefinition& p) {
    return (&p == G4Electron::Electron() or &p == G4Positron::Positron());
}

void G4eDarkBremsstrahlung::InitialiseEnergyLossProcess(const G4ParticleDefinition*,
                                                        const G4ParticleDefinition*) {
    if(!isInitialised) {
        theModel_ = new G4eDarkBremsstrahlungModel();
        SetEmModel(theModel_,1);

        G4double energyLimit = 1*GeV;

        EmModel(1)->SetLowEnergyLimit(MinKinEnergy());
        EmModel(1)->SetHighEnergyLimit(energyLimit);
        
        G4VEmFluctuationModel* fm = 0;
        AddEmModel(1, EmModel(1), fm);

        isInitialised = true;
    }

    G4double eth = 0*MeV;
    EmModel(1)->SetSecondaryThreshold(eth);
    EmModel(1)->SetLPMFlag(false);
}

void G4eDarkBremsstrahlung::SetMethod(std::string method_in) {
    ((G4eDarkBremsstrahlungModel*)EmModel(1))->SetMethod(method_in);
    return;
}
