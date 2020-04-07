/**
 * @file G4eDarkBremsstrahlung.cxx
 * @brief Class providing the Dark Bremsstrahlung process class.
 * @author Michael Revering, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/G4eDarkBremsstrahlung.h"
#include "SimCore/G4APrime.h"

#include "G4DynamicParticle.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4ProcessType.hh"

G4eDarkBremsstrahlung::G4eDarkBremsstrahlung(const G4String& name):
    G4VEnergyLossProcess(name),
    isInitialised(false) {  

    G4int subtype = 63;   
    SetProcessSubType(subtype);
    SetSecondaryParticle(G4APrime::APrime());
    SetIonisation(false);
}

G4bool G4eDarkBremsstrahlung::IsApplicable(const G4ParticleDefinition& p) {
    return &p == G4Electron::Electron();
}

void G4eDarkBremsstrahlung::PrintInfo() {

    std::string method = "UNDEFINED";
    if ( method_ == G4eDarkBremsstrahlungModel::DarkBremMethod::ForwardOnly ) {
        method = "forward_only";
    } else if ( method_ == G4eDarkBremsstrahlungModel::DarkBremMethod::CMScaling ) {
        method = "cm_scaling";
    }

    G4cout << "\tInterpretation Method: " + method << G4endl;
    G4cout << "\tMad Graph Data File  : " + madGraphFile_ << G4endl;

}

void G4eDarkBremsstrahlung::InitialiseEnergyLossProcess(const G4ParticleDefinition*,
                                                        const G4ParticleDefinition*) {
    if(!isInitialised) {

        this->SetEmModel(new G4eDarkBremsstrahlungModel(),0); //adds model to vector stored in process

        //TODO: could make this depend on maximum beam energy passed through LHE files?
        G4double energyLimit = 4*GeV;

        this->EmModel(0)->SetLowEnergyLimit(MinKinEnergy());
        this->EmModel(0)->SetHighEnergyLimit(energyLimit);
        dynamic_cast<G4eDarkBremsstrahlungModel*>(this->EmModel(0))->SetMethod( method_ );
        dynamic_cast<G4eDarkBremsstrahlungModel*>(this->EmModel(0))->SetMadGraphDataFile( madGraphFile_ );
        
        G4VEmFluctuationModel* fm = 0;
        //adds model to ModelManager which handles initialisation procedures and cleaning up pointers
        this->AddEmModel(0, EmModel(0), fm); 

        isInitialised = true;
    }

    G4double eth = 0*MeV;
    this->EmModel(0)->SetSecondaryThreshold(eth);
    this->EmModel(0)->SetLPMFlag(false);
}

void G4eDarkBremsstrahlung::SetMethod(G4eDarkBremsstrahlungModel::DarkBremMethod method) {
    method_ = method;
    return;
}

void G4eDarkBremsstrahlung::SetMadGraphDataFile(std::string file) {
    madGraphFile_ = file;
    return;
}
