/*
 * DetectorServiceImpl.h
 * @brief
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORSERVICEIMPL_H_
#define DETDESCR_DETECTORSERVICEIMPL_H_

#include "DetDescr/DetectorElement.h"
#include "DetDescr/DetectorDataService.h"

#include <iostream>
#include <cstdlib>
#include <dirent.h>

namespace ldmx {

    class DetectorDataServiceImpl : public DetectorDataService {

        public:

            typedef std::map<std::string, std::string> DetectorAliasMap;

            virtual ~DetectorDataServiceImpl() {;}

            DetectorDataServiceImpl() {
                setupLocalAliases();
            }

            TGeoManager* getGeometryManager() {
                return geoManager_;
            }

            DetectorElement* getTopDetectorElement() {
                return topDE_;
            }

            void initialize() {

                if (detectorName_.empty()) {
                    throw std::runtime_error("The detector name was not set.");
                }

                if (aliasMap_.find(detectorName_) == aliasMap_.end()) {
                    std::cerr << "ERROR: There is no entry for the detector name "
                            << detectorName_ << " in the aliases." << std::endl;
                    throw std::runtime_error("Detector name was not found in the aliases.");
                }

                std::string location = aliasMap_[detectorName_];

                std::cout << "[ DetectorDataServiceImpl ] : Importing detector " << detectorName_
                        << " from " << location << std::endl;

                geoManager_ = TGeoManager::Import(location.c_str(), detectorName_.c_str());

                // call setup on top DE which should recursively setup sub-detectors

                geoManager_->CloseGeometry();
            }

            void setDetectorName(std::string detectorName) {
                detectorName_ = detectorName;
            }

            void addAlias(std::string detectorName, std::string alias) {
                aliasMap_[detectorName] = alias;
                std::cout << "[ DetectorDataServiceImpl ] : Added alias " << detectorName << " => " << alias << std::endl;
            }

        private:

            /**
             * Build a cache of detector aliases from names to their locations on
             * disk, using the LDMXSW_DIR environment variable to locate the main
             * detector data directory in the installation area.
             */
            void setupLocalAliases();

        private:

            std::string detectorName_;
            DetectorAliasMap aliasMap_;
            TGeoManager* geoManager_{nullptr};
            DetectorElement* topDE_{nullptr};
    };
}

#endif /* INCLUDE_DETDESCR_DETECTORSERVICEIMPL_H_ */
