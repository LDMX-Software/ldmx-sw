#include "SimPlugins/TrackFilterMessenger.h"

#include "SimPlugins/TrackFilterPlugin.h"
#include "SimCore/TrackFilter.h"

namespace ldmx {

    TrackFilterMessenger::TrackFilterMessenger(TrackFilterPlugin* plugin)
            : UserActionPluginMessenger(plugin), plugin_(plugin) {

        thresholdCmd_ = new G4UIcmdWithADoubleAndUnit(std::string(getPath() + "threshold").c_str(), this);
        thresholdCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);

        pdgCodeCmd_ = new G4UIcmdWithAnInteger(std::string(getPath() + "pdgid").c_str(), this);
        pdgCodeCmd_->SetGuidance("Add a PDG ID code for saving tracks");
        pdgCodeCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);

        particleCmd_ = new G4UIcmdWithAString(std::string(getPath() + "particle").c_str(), this);
        particleCmd_->SetGuidance("Add the name of a particle for saving tracks");
        particleCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        G4UIparameter* particle = new G4UIparameter("particleName", 's', false);
        particle->SetGuidance("Name of particle");
        particleCmd_->SetParameter(particle);

        processCmd_ = new G4UIcommand(std::string(getPath() + "process").c_str(), this);
        processCmd_->SetGuidance("Add a creator physics process name for saving tracks.");
        processCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        G4UIparameter* processName = new G4UIparameter("processName", 's', false);
        processName->SetGuidance("Name of Geant4 physics process to save");
        processCmd_->SetParameter(processName);
        G4UIparameter* exactMatch = new G4UIparameter("exactMatch", 'b', true);
        exactMatch->SetGuidance("True if process name match should be exact");
        processCmd_->SetParameter(exactMatch);

        parentCmd_ = new G4UIcommand(std::string(getPath() + "parent").c_str(), this);
        parentCmd_->SetGuidance("Add a physics process name of daughter for saving its parent particle");
        parentCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        processName = new G4UIparameter("processName", 's', false);
        processName->SetGuidance("Name of Geant4 physics process to save");
        parentCmd_->SetParameter(processName);
        exactMatch = new G4UIparameter("exactMatch", 'b', true);
        exactMatch->SetGuidance("True if process name match should be exact");
        parentCmd_->SetParameter(exactMatch);

        regionCmd_ = new G4UIcommand(std::string(getPath() + "region").c_str(), this);
        regionCmd_->SetGuidance("Add a detector region for saving tracks");
        regionCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        G4UIparameter* regionName = new G4UIparameter("regionName", 's', false);
        regionName->SetGuidance("Name of region");
        regionCmd_->SetParameter(regionName);
        G4UIparameter* regionSave = new G4UIparameter("save", 'b', true);
        regionSave->SetGuidance("True to save tracks in the region or false to not save");
        regionCmd_->SetParameter(regionSave);

        volumeCmd_ = new G4UIcommand(std::string(getPath() + "volume").c_str(), this);
        volumeCmd_->SetGuidance("Add a detector volume for saving tracks");
        volumeCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
        G4UIparameter* volumeName = new G4UIparameter("volumeName", 's', false);
        volumeName->SetGuidance("Name of volume");
        volumeCmd_->SetParameter(volumeName);
        G4UIparameter* volumeSave = new G4UIparameter("save", 'b', true);
        volumeSave->SetGuidance("True to save tracks in the volume or false to not save");
        volumeCmd_->SetParameter(volumeSave);

        createCmd_ = new G4UIcmdWithAString(std::string(getPath() + "create").c_str(), this);
        createCmd_->SetGuidance("Add the current set of filters as a named filter chain");
        createCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);

        printCmd_ = new G4UIcmdWithAString(std::string(getPath() + "print").c_str(), this);
        printCmd_->GetParameter(0)->SetOmittable(true);
        createCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);

        actionCmd_ = new G4UIcmdWithAString(std::string(getPath() + "action").c_str(), this);
        actionCmd_->SetGuidance("Set the tracking action for the filters ('pre' or 'post')");
        actionCmd_->AvailableForStates(G4ApplicationState::G4State_Idle);
    }

    void TrackFilterMessenger::SetNewValue(G4UIcommand *cmd, G4String newValue) {

        UserActionPluginMessenger::SetNewValue(cmd, newValue);

        if (cmd == thresholdCmd_) {
            double threshold = thresholdCmd_->GetNewDoubleValue(newValue);
            filters_.push_back(new TrackEnergyFilter(threshold));
        } else if (cmd == pdgCodeCmd_) {
            int pdgid = pdgCodeCmd_->GetNewIntValue(newValue);
            TrackPDGCodeFilter* pdgFilter = getFilter<TrackPDGCodeFilter>();
            pdgFilter->addPDGCode(pdgid);
        } else if (cmd == processCmd_ || cmd == parentCmd_) {
            std::stringstream ss(newValue);
            std::string processName;
            bool exactMatch = false;
            ss >> processName;
            if (!ss.eof()) {
                std::string exactMatchStr;
                ss >> exactMatchStr;
                exactMatch = G4UIcommand::ConvertToBool(exactMatchStr.c_str());
            }
            if (cmd == processCmd_) {
                TrackProcessFilter* processFilter = getFilter<TrackProcessFilter>();
                processFilter->addProcess(processName, exactMatch);
            } else if (cmd == parentCmd_) {
                TrackParentProcessFilter* parentFilter = getFilter<TrackParentProcessFilter>();
                parentFilter->addProcess(processName, exactMatch);
            }
        } else if (cmd == regionCmd_) {
            TrackRegionFilter* regionFilter = getFilter<TrackRegionFilter>();
            std::stringstream ss(newValue);
            std::string regionName;
            bool regionSave = true;
            ss >> regionName;
            if (!ss.eof()) {
                std::string regionSaveStr;
                ss >> regionSaveStr;
                regionSave = G4UIcommand::ConvertToBool(regionSaveStr.c_str());
            }
            regionFilter->addRegion(regionName, regionSave);
        } else if (cmd == volumeCmd_) {
            TrackVolumeFilter* volumeFilter = getFilter<TrackVolumeFilter>();
            std::stringstream ss(newValue);
            std::string volumeName;
            bool volumeSave = true;
            ss >> volumeName;
            if (!ss.eof()) {
                std::string volumeSaveStr;
                ss >> volumeSaveStr;
                volumeSave = G4UIcommand::ConvertToBool(volumeSaveStr.c_str());
            }
            volumeFilter->addVolume(volumeName, volumeSave);
        } else if (cmd == particleCmd_) {
            getFilter<TrackPDGCodeFilter>()->addParticle(newValue);
        } else if (cmd == actionCmd_) {
            std::string newAction(newValue);
            std::transform(newAction.begin(), newAction.end(), newAction.begin(), ::tolower);
            if (newAction != "pre" && newAction != "post") {
                G4Exception("", "", JustWarning, std::string("Unknown tracking action: " + newValue).c_str());
            } else {
                action_ = newAction;
            }
        } else if (cmd == createCmd_) {
            TrackFilterChain* filterChain = new TrackFilterChain(newValue);
            for (auto filter : filters_) {
                filterChain->addFilter(filter);
            }
            if (action_ == "pre") {
                plugin_->addPreFilterChain(filterChain);
            } else {
                plugin_->addPostFilterChain(filterChain);
            }
            filters_.clear();
        } else if (cmd == printCmd_) {
            if (newValue.size() != 0) {
                TrackFilterChain* filterChain = plugin_->getFilterChain(newValue);
                if (filterChain) {
                    plugin_->print(std::cout, filterChain);
                }
            } else {
                plugin_->print(std::cout);
            }
        }
    }
}
