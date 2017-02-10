#include "DetDescr/DetectorDataServiceImpl.h"

#include "DetDescr/TopDetectorElement.h"

namespace ldmx {

    void DetectorDataServiceImpl::setupLocalAliases() {
        const char* envDir = std::getenv("LDMXSW_DIR");
        if (!envDir) {
            // FIXME: Should use base Exception class here.
            throw std::runtime_error("The LDMXSW_DIR env variable is not set.");
        }
        std::string baseDir = std::string(envDir) + "/data/detectors";
        DIR* dir = opendir(baseDir.c_str());
        struct dirent* entry = readdir(dir);
        while (entry != nullptr) {
            if (entry->d_type == DT_DIR) {
                std::string detectorName = entry->d_name;
                if (detectorName != "." && detectorName != "..") {
                    // "file://" +
                    std::string location = baseDir + "/" + detectorName + "/detector_full.gdml";
                    addAlias(detectorName, location);
                }
            }
            entry = readdir(dir);
        }

    }

    void DetectorDataServiceImpl::initialize() {

        // Throw an error if detector name is not set.
        if (detectorName_.empty()) {
            // FIXME: Should use base Exception class here.
            throw std::runtime_error("The detector name was not set.");
        }

        // Throw an error if there is no known alias (location) for this detector.
        if (aliasMap_.find(detectorName_) == aliasMap_.end()) {
            // FIXME: Should use base Exception class here.
            std::cerr << "ERROR: There is no entry for the detector name "
                    << detectorName_ << " in the aliases." << std::endl;
            throw std::runtime_error("Detector name was not found in the aliases.");
        }

        // Get the location on disk of the detector name (presumably a GDML file).
        std::string location = aliasMap_[detectorName_];

        std::cout << "[ DetectorDataServiceImpl ] : Importing detector " << detectorName_
                << " from " << location << std::endl;

        // Import the geometry into ROOT.
        geoManager_ = TGeoManager::Import(location.c_str(), detectorName_.c_str());

        // Setup the top DE which will setup subdetector components.
        topDE_ = new TopDetectorElement(geoManager_->GetTopNode());
    }
}
