#include "SimPlugins/TrackFilterMessenger.h"

#include "SimPlugins/TrackFilterPlugin.h"

#include "G4UIcmdWithADoubleAndUnit.hh"

namespace ldmx {

    TrackFilterMessenger::TrackFilterMessenger(TrackFilterPlugin* plugin)
            : UserActionPluginMessenger(plugin), plugin_(plugin) {

        thresholdCmd_ = new G4UIcmdWithADoubleAndUnit(std::string(getPath() + "threshold").c_str(), this);
        thresholdCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);

        pdgCodeCmd_ = new G4UIcmdWithAnInteger(std::string(getPath() + "pdgid").c_str(), this);
        pdgCodeCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);

        processCmd_ = new G4UIcommand(std::string(getPath() + "process").c_str(), this);
        processCmd_->SetGuidance("Add a creator physics process name for saving tracks.");
        processCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
        G4UIparameter* processName = new G4UIparameter("processName", 's', false);
        processName->SetGuidance("Name of Geant4 physics process to save");
        processCmd_->SetParameter(processName);
        G4UIparameter* exactMatch = new G4UIparameter("exactMatch", 'b', true);
        exactMatch->SetGuidance("True if process name match should be exact");
        processCmd_->SetParameter(exactMatch);

        parentCmd_ = new G4UIcommand(std::string(getPath() + "parent").c_str(), this);
        parentCmd_->SetGuidance("Add a physics process name of daughter for saving its parent particle");
        parentCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
        processName = new G4UIparameter("processName", 's', false);
        processName->SetGuidance("Name of Geant4 physics process to save");
        parentCmd_->SetParameter(processName);
        exactMatch = new G4UIparameter("exactMatch", 'b', true);
        exactMatch->SetGuidance("True if process name match should be exact");
        parentCmd_->SetParameter(exactMatch);

        regionCmd_ = new G4UIcommand(std::string(getPath() + "region").c_str(), this);
        regionCmd_->SetGuidance("Add a detector region for saving tracks");
        regionCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
        G4UIparameter* regionName = new G4UIparameter("regionName", 's', false);
        regionName->SetGuidance("Name of region");
        regionCmd_->SetParameter(regionName);
        G4UIparameter* regionSave = new G4UIparameter("save", 'b', true);
        regionSave->SetGuidance("True to save tracks in the region or false to not save");
        regionCmd_->SetParameter(regionSave);

        createCmd_ = new G4UIcmdWithAString(std::string(getPath() + "create").c_str(), this);
        createCmd_->AvailableForStates(G4ApplicationState::G4State_PreInit, G4ApplicationState::G4State_Idle);
    }

    void TrackFilterMessenger::SetNewValue(G4UIcommand *cmd, G4String newValue) {

        UserActionPluginMessenger::SetNewValue(cmd, newValue);

        if (cmd == thresholdCmd_) {
            double threshold = thresholdCmd_->GetNewDoubleValue(newValue);
            filters_.push_back(new TrackEnergyFilter(threshold));
        } else if (cmd == pdgCodeCmd_) {
            int pdgid = pdgCodeCmd_->GetNewIntValue(newValue);
            TrackPDGCodeFilter* pdgFilter = nullptr;
            for (auto filter : filters_) {
                if (dynamic_cast<TrackPDGCodeFilter*>(filter)) {
                    pdgFilter = dynamic_cast<TrackPDGCodeFilter*>(filter);
                }
            }
            if (!pdgFilter) {
                pdgFilter = new TrackPDGCodeFilter;
                filters_.push_back(pdgFilter);
            }
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
                filters_.push_back(new TrackProcessFilter(processName, exactMatch));
            } else if (cmd == parentCmd_) {
                filters_.push_back(new TrackParentProcessFilter(processName, exactMatch));
            }
        } else if (cmd == createCmd_) {
            TrackFilterChain* filterChain = new TrackFilterChain(newValue);
            for (auto filter : filters_) {
                filterChain->addFilter(filter);
            }
            plugin_->addFilterChain(filterChain);
            filters_.clear();
        } else if (cmd == regionCmd_) {
            TrackRegionFilter* regionFilter = nullptr;
            for (auto filter : filters_) {
                if (dynamic_cast<TrackRegionFilter*>(filter)) {
                    regionFilter = dynamic_cast<TrackRegionFilter*>(filter);
                }
            }
            if (!regionFilter) {
                regionFilter = new TrackRegionFilter;
                filters_.push_back(regionFilter);
            }
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
        } 
    }
}
