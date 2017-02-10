#include "DetDescr/DetectorDataServiceImpl.h"

namespace ldmx {

void DetectorDataServiceImpl::setupLocalAliases() {
    const char* envDir = std::getenv("LDMXSW_DIR");
    if (!envDir) {
        // FIXME: Should use base Exception class.
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

}
