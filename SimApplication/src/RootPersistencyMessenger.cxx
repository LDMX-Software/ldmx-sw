#include "SimApplication/RootPersistencyMessenger.h"

<<<<<<< HEAD
//----------------//
//   C++ StdLib   //
//----------------//
#include <string>

//------------//
//   Geant4   //
//------------//
#include "G4UIdirectory.hh"
#include "G4UIparameter.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithAString.hh"

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimApplication/RootPersistencyManager.h"

=======
// STL
#include <string>

>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
namespace ldmx {

    RootPersistencyMessenger::RootPersistencyMessenger(RootPersistencyManager* rootIO) :
            rootIO_(rootIO) {

        rootDir_ = new G4UIdirectory("/ldmx/persistency/root/");
        rootDir_->SetGuidance("ROOT persistency commands for writing events");

        rootFileCmd_ = new G4UIcommand("/ldmx/persistency/root/file", this);
        G4UIparameter* filename = new G4UIparameter("filename", 's', false);
        rootFileCmd_->SetParameter(filename);
        rootFileCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        rootFileCmd_->SetGuidance("Set the ROOT output file name.");

        verboseCmd_ = new G4UIcommand("/ldmx/persistency/root/verbose", this);
        G4UIparameter* verboseLevel = new G4UIparameter("verboseLevel", 'i', false);
        verboseCmd_->SetParameter(verboseLevel);
        verboseCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        verboseCmd_->SetGuidance("Set the ROOT IO verbose level.");

        disableCmd_ = new G4UIcommand("/ldmx/persistency/root/disable", this);
        disableCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        disableCmd_->SetGuidance("Disable ROOT IO.");

        enableCmd_ = new G4UIcommand("/ldmx/persistency/root/enable", this);
        enableCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        enableCmd_->SetGuidance("Re-enable ROOT IO after it has been disabled.");

        comprCmd_ = new G4UIcommand("/ldmx/persistency/root/compression", this);
        G4UIparameter* compLevel = new G4UIparameter("compressionLevel", 'i', false);
        comprCmd_->SetParameter(compLevel);
        comprCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        comprCmd_->SetGuidance("Set the output file compression level (1-9).");

        hitContribsCmd_ = new G4UIcmdWithABool("/ldmx/persistency/root/enableHitContribs", this);
        G4UIparameter* enable = new G4UIparameter("enable", 'b', true);
        hitContribsCmd_->SetParameter(enable);
        hitContribsCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        hitContribsCmd_->SetGuidance("Enable hit contributions for SimCalorimeterHits (on by default)");

        compressContribsCmd_ = new G4UIcmdWithABool("/ldmx/persistency/root/compressHitContribs", this);
        G4UIparameter* compress = new G4UIparameter("enable", 'b', true);
        compressContribsCmd_->SetParameter(compress);
        compressContribsCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        compressContribsCmd_->SetGuidance("Compress hit contributions by matching SimParticle and PDG code");
    
        dropCmd_ = new G4UIcmdWithAString{"/ldmx/persistency/root/dropCol", this}; 
        dropCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        dropCmd_->SetGuidance("Drop the hits associated with the specified collection."); 
<<<<<<< HEAD
    
        descriptionCmd_ = new G4UIcmdWithAString{"/ldmx/persistency/root/description", this};
        descriptionCmd_->SetGuidance("Description of this run.");  

        runCmd_ = new G4UIcmdWithAnInteger{"/ldmx/persistency/root/runNumber", this}; 
        runCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        runCmd_->SetGuidance("Set the run number."); 
=======
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
    }

    RootPersistencyMessenger::~RootPersistencyMessenger() {
        delete rootFileCmd_;
        delete verboseCmd_;
        delete enableCmd_;
        delete disableCmd_;
        delete comprCmd_;
<<<<<<< HEAD
        delete rootDir_;
        delete dropCmd_;
        delete descriptionCmd_; 
        delete runCmd_;  
=======
        //delete modeCmd_;
        delete rootDir_;
        delete dropCmd_; 
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
    }

    void RootPersistencyMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {

        // Is ROOT IO enabled?
        if (RootPersistencyManager::getInstance()) {
            if (command == rootFileCmd_) {
                rootIO_->setFileName(newValues);
            } else if (command == verboseCmd_) {
                int verboseLevel = std::stoi(newValues);
                if (verboseLevel > 4) {
                    verboseLevel = 4;
                } else if (verboseLevel < 1) {
                    verboseLevel = 1;
                }
                rootIO_->SetVerboseLevel(verboseLevel);
            } else if (command == disableCmd_) {
                G4PersistencyCenter::GetPersistencyCenter()->DeletePersistencyManager();
                rootIO_ = nullptr;
            } else if (command == comprCmd_) {
                int compr = std::stoi(newValues);
                rootIO_->setCompressionLevel(compr);
<<<<<<< HEAD
            } else if (command == hitContribsCmd_) {
                rootIO_->setEnableHitContribs(
                        static_cast<G4UIcmdWithABool*>(hitContribsCmd_)->GetNewBoolValue(newValues.c_str()));
            } else if (command == compressContribsCmd_) {
                rootIO_->setCompressHitContribs(
                        static_cast<G4UIcmdWithABool*>(compressContribsCmd_)->GetNewBoolValue(newValues.c_str()));
            } else if (command == dropCmd_) { 
                rootIO_->dropCollection(newValues); 
            } else if (command == descriptionCmd_) {
                rootIO_->setRunDescription(newValues);  
            } else if (command == runCmd_) { 
                rootIO_->setRunNumber(
                       static_cast<G4UIcmdWithAnInteger*>(runCmd_)->GetNewIntValue(
                           newValues.c_str())); 
=======
            } /*else if (command == modeCmd_) {
             rootIO_->getWriter()->setMode(newValues);
             }*/else if (command == hitContribsCmd_) {
                rootIO_->setEnableHitContribs(hitContribsCmd_->GetNewBoolValue(newValues.c_str()));
            } else if (command == compressContribsCmd_) {
                rootIO_->setCompressHitContribs(compressContribsCmd_->GetNewBoolValue(newValues.c_str()));
            } else if (command == dropCmd_) { 
                rootIO_->dropCollection(newValues); 
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
            }
        } else {
            // Re-enable ROOT IO.
            if (command == enableCmd_) {
                rootIO_ = new RootPersistencyManager();
            } else {
                // Print a warning if user tries to send commands when IO is disabled.
                std::cout << "WARNING: Command was ignored.  ROOT IO is disabled!" << std::endl;
            }
        }
    }

} // namespace sim
