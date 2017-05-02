#include "SimApplication/PrimaryGeneratorMessenger.h"

// LDMX
#include "SimApplication/LHEPrimaryGenerator.h"
#include "SimApplication/RootPrimaryGenerator.h"
#include "SimApplication/MultiParticleGunPrimaryGenerator.h"

namespace ldmx {

    bool PrimaryGeneratorMessenger::useRootSeed_{false};
    std::string PrimaryGeneratorMessenger::particleType_{"e-"};
    double PrimaryGeneratorMessenger::particleEnergy_{4.0};
    int PrimaryGeneratorMessenger::nInteractions_{1};

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

        rootUseSeedCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);

    }

    PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {;}

    void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {

        if      (command == lheOpenCmd_)           { primaryGeneratorAction_->setPrimaryGenerator(new LHEPrimaryGenerator(new LHEReader(newValues))); }
        else if (command == rootOpenCmd_)          { primaryGeneratorAction_->setPrimaryGenerator(new RootPrimaryGenerator(newValues)); }
        else if (command == rootUseSeedCmd_)       { useRootSeed_ = true; }
        else if (command == mpgunEnergyCmd_)       { particleEnergy_ = G4UIcommand::ConvertToDouble(newValues); }
        else if (command == mpgunNIntCmd_)         { nInteractions_ = G4UIcommand::ConvertToInt(newValues); }
        else if (command == mpgunParticleTypeCmd_) { particleType_ = newValues; }
        else if (command == enableMPGunCmd_) {
            std::cout << "we are using the multiparitcle gun!" << std::endl;
            primaryGeneratorAction_->setPrimaryGenerator(new MultiParticleGunPrimaryGenerator());
        }

    }

}
