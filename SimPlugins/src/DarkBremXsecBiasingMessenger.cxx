/**
 * @file DarkBremXsecBiasingMessenger.cxx
 * @brief Class providing a macro messenger for a DarkBremXsecBiasingPlugin
 * @author Michael Revering, University of Minnesota
 */

#include "SimPlugins/DarkBremXsecBiasingMessenger.h"

// LDMX
#include "Exception/Exception.h"
#include "SimPlugins/DarkBremXsecBiasingPlugin.h"

// STL
#include <string>

namespace ldmx {

    DarkBremXsecBiasingMessenger::DarkBremXsecBiasingMessenger(DarkBremXsecBiasingPlugin* biasingPlugin) :
            UserActionPluginMessenger(biasingPlugin), biasingPlugin_(biasingPlugin) {

        xsecFactorCmd_ = new G4UIcommand(std::string(getPath() + "xsecFactor").c_str(), this);
        G4UIparameter* modulus = new G4UIparameter("xsecFactor", 'd', false);
        xsecFactorCmd_->SetParameter(modulus);
        xsecFactorCmd_->SetGuidance("Set the cross section biasing factor for the Dark Brem process.");
        xsecFactorCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
    
        modeCmd_ = new G4UIcommand(std::string(getPath() + "mode").c_str(), this);
        G4UIparameter* mode = new G4UIparameter("mode", 's', false);
        modeCmd_->SetParameter(mode);
        modeCmd_->SetGuidance("Set the simulation mode for the Dark Brem process. Options are \"forward_only\" or \"cm_scaling\".");
        modeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);

        madGraphDataFileCmd_ = new G4UIcommand(std::string(getPath() + "madGraphDataFile").c_str(), this);
        G4UIparameter* madGraphDataFile = new G4UIparameter("madGraphDataFile", 's', false);
        madGraphDataFileCmd_->SetParameter(madGraphDataFile);
        madGraphDataFileCmd_->SetGuidance(
                "Set the LHE file that contains mad graph data on dark brem processes. Needed for re-scaling." );
        madGraphDataFileCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
    }

    void DarkBremXsecBiasingMessenger::SetNewValue(G4UIcommand *command, G4String newValue) {

        // Handles verbose command.
        UserActionPluginMessenger::SetNewValue(command, newValue);

        if (command == xsecFactorCmd_) {
            biasingPlugin_->setXsecBiasingFactor(std::stod(newValue));
        } else if (command == modeCmd_) {
            if( newValue == "forward_only" ) {
                biasingPlugin_->setXsecSimulationMethod(
                        G4eDarkBremsstrahlungModel::DarkBremMethod::ForwardOnly );
            } else if( newValue == "cm_scaling" ) {
                biasingPlugin_->setXsecSimulationMethod(
                        G4eDarkBremsstrahlungModel::DarkBremMethod::CMScaling );
            } else {
                EXCEPTION_RAISE(
                        "InvalidMethod",
                        "Input method '" + newValue + "' is not a valid dark brem simulation method." );
            }
        } else if (command == madGraphDataFileCmd_ ) {
            biasingPlugin_->setMadGraphDataFile( newValue );
        }

    } //SetNewValue

} // namespace ldmx
