/**
 * @file PrimaryGeneratorMessenger.cxx
 * @brief Class providing a macro messenger for event generation
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#include "SimApplication/PrimaryGeneratorMessenger.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimApplication/GeneralParticleSource.h"
#include "SimApplication/LHEPrimaryGenerator.h"
#include "SimApplication/MultiParticleGunPrimaryGenerator.h"
#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/RootPrimaryGenerator.h"


namespace ldmx {

    bool PrimaryGeneratorMessenger::useRootSeed_{false};

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

        //
        // GPS commands
        //

        // Create a macro directory for GPS commands.
        gpsDir_ = std::make_unique<G4UIdirectory>("/ldmx/generators/gps/");
        gpsDir_->SetGuidance("Command for GPS generation."); 
        
        // Command to enable the use of GPS
        enableGPSCmd_ = std::make_unique<G4UIcommand>("/ldmx/generators/gps/enable", this);  
        enableGPSCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
    }


    PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {;}

    void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {

        ///////// LHE input
        if (command == lheOpenCmd_) { 
            primaryGeneratorAction_->setPrimaryGenerator(new LHEPrimaryGenerator(new LHEReader(newValues))); 
        }
    
        ///////// ROOT input
        else if (command == rootOpenCmd_) { 
            primaryGeneratorAction_->setPrimaryGenerator(new RootPrimaryGenerator(newValues)); 
        } else if (command == rootUseSeedCmd_) {
            useRootSeed_ = true; 
        } else if (command == rootEcalSPCmd_) {
            int curi = primaryGeneratorAction_->getIndexRPG();
            if (curi >= 0) ( dynamic_cast<RootPrimaryGenerator*>(primaryGeneratorAction_->getGenerator(curi)) )->setRunMode( 1 ); 
        } else if (command == rootRegenCmd_) {
            int curi = primaryGeneratorAction_->getIndexRPG();
            if (curi >= 0) ( dynamic_cast<RootPrimaryGenerator*>(primaryGeneratorAction_->getGenerator(curi)) )->setRunMode( 0 ); 
        }

        //////// MPGun commands
        else if (command == enableMPGunCmd_) { 
            primaryGeneratorAction_->setPrimaryGenerator(new MultiParticleGunPrimaryGenerator());
        } else if (command == mpgunNParCmd_) { 
            int curi = primaryGeneratorAction_->getIndexMPG();
            if (curi >= 0) ( dynamic_cast<MultiParticleGunPrimaryGenerator*>(primaryGeneratorAction_->getGenerator(curi)) )->setMpgNparticles( G4UIcommand::ConvertToDouble(newValues) ); 
        } else if (command == enableMPGunPoissonCmd_) { 
            int curi = primaryGeneratorAction_->getIndexMPG();
            if (curi >= 0) ( dynamic_cast<MultiParticleGunPrimaryGenerator*>(primaryGeneratorAction_->getGenerator(curi)) )->enablePoisson(); 
        } else if (command == mpgunPIDCmd_) { 
            int curi = primaryGeneratorAction_->getIndexMPG();
            if (curi >= 0) ( dynamic_cast<MultiParticleGunPrimaryGenerator*>(primaryGeneratorAction_->getGenerator(curi)) )->setMpgPdgId( G4UIcommand::ConvertToInt(newValues) ); 
        } else if (command == mpgunVtxCmd_) { 
            int curi = primaryGeneratorAction_->getIndexMPG();
            if (curi >= 0) ( dynamic_cast<MultiParticleGunPrimaryGenerator*>(primaryGeneratorAction_->getGenerator(curi)) )->setMpgVertex( G4UIcommand::ConvertToDimensioned3Vector(newValues) ); 
        } else if (command == mpgunMomCmd_) { 
            int curi = primaryGeneratorAction_->getIndexMPG();
            if (curi >= 0) ( dynamic_cast<MultiParticleGunPrimaryGenerator*>(primaryGeneratorAction_->getGenerator(curi)) )->setMpgMomentum( G4UIcommand::ConvertToDimensioned3Vector(newValues) ); 
        }        

        //////// Beamspot commands    
        else if (command == enableBeamspotCmd_) { 
            primaryGeneratorAction_->setUseBeamspot(true); 
        } else if (command == beamspotXSizeCmd_) {
            primaryGeneratorAction_->setBeamspotXSize( G4UIcommand::ConvertToDouble(newValues) ); 
        } else if (command == beamspotYSizeCmd_) { 
            primaryGeneratorAction_->setBeamspotYSize( G4UIcommand::ConvertToDouble(newValues) ); 
        } else if (command == beamspotZSizeCmd_) { 
            primaryGeneratorAction_->setBeamspotZSize( G4UIcommand::ConvertToDouble(newValues) ); 
        }

        //
        // GPS
        //
        else if (command == enableGPSCmd_.get()) { 
            primaryGeneratorAction_->setPrimaryGenerator(new GeneralParticleSource());  
        }
    }
}
