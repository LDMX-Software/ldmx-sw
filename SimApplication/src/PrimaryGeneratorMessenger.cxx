#include "SimApplication/PrimaryGeneratorMessenger.h"

// LDMX
#include "SimApplication/LHEPrimaryGenerator.h"
#include "SimApplication/RootPrimaryGenerator.h"

namespace ldmx {

PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGeneratorAction* thePrimaryGeneratorAction) :
    primaryGeneratorAction_(thePrimaryGeneratorAction) {

    // lhe commands
    lheDir_ = new G4UIdirectory("/ldmx/generators/lhe/");
    lheDir_->SetGuidance("Commands for LHE event generation");

    lheOpenCmd_ = new G4UIcommand("/ldmx/generators/lhe/open", this);
    G4UIparameter* lhefilename = new G4UIparameter("filename", 's', true);
    lheOpenCmd_->SetParameter(lhefilename);
    lheOpenCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);

    // root commands
    rootDir_ = new G4UIdirectory("/ldmx/generators/root/");
    rootDir_->SetGuidance("Commands for ROOT event generation");

    rootOpenCmd_ = new G4UIcommand("/ldmx/generators/root/open", this);
    G4UIparameter* rootfilename = new G4UIparameter("filename", 's', true);
    rootOpenCmd_->SetParameter(rootfilename);
    rootOpenCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);

}

PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {

}

void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
    if (command == lheOpenCmd_) {
        primaryGeneratorAction_->setPrimaryGenerator(
                new LHEPrimaryGenerator(new LHEReader(newValues)));
    }
    if (command == rootOpenCmd_) {
        primaryGeneratorAction_->setPrimaryGenerator(
                new RootPrimaryGenerator( newValues ) );
    }    
}

}
