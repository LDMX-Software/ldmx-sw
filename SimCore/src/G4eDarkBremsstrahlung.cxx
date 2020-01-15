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

    //Setting the process type to user defined forces some G4 magic to
    // happen that allows this process to be persisted through initialisation
    SetProcessType( G4ProcessType::fUserDefined ); //This is really important

    G4int subtype = 63;   
    SetProcessSubType(subtype);
    SetSecondaryParticle(G4APrime::APrime());
    SetIonisation(false);
}

G4bool G4eDarkBremsstrahlung::IsApplicable(const G4ParticleDefinition& p) {
    return &p == G4Electron::Electron();
}

void G4eDarkBremsstrahlung::InitialiseEnergyLossProcess(const G4ParticleDefinition*,
                                                        const G4ParticleDefinition*) {
    if(!isInitialised) {

        this->SetEmModel(new G4eDarkBremsstrahlungModel(),0); //adds model to vector stored in process

        G4double energyLimit = 1*GeV;

        this->EmModel(0)->SetLowEnergyLimit(MinKinEnergy());
        this->EmModel(0)->SetHighEnergyLimit(energyLimit);
        
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
    dynamic_cast<G4eDarkBremsstrahlungModel *>(this->EmModel(0))->SetMethod(method);
    return;
}

void G4eDarkBremsstrahlung::SetMadGraphDataFile(std::string file) {
    dynamic_cast<G4eDarkBremsstrahlungModel *>(this->EmModel(0))->SetMadGraphDataFile(file);
    return;
}
