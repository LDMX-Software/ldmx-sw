/*
 * DetectorServiceImpl.h
 * @brief Simple detector service implementation for loading GDML files into ROOT
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORSERVICEIMPL_H_
#define DETDESCR_DETECTORSERVICEIMPL_H_

#include "DetDescr/DetectorElementImpl.h"
#include "DetDescr/DetectorDataService.h"

#include <iostream>
#include <cstdlib>
#include <dirent.h>

namespace ldmx {

    /**
     * @class DetectorDataServiceImpl
     * @brief Implements a simple service for loading GDML files into ROOT
     */
    class DetectorDataServiceImpl : public DetectorDataService {

        public:

            typedef std::map<std::string, std::string> DetectorAliasMap;

            virtual ~DetectorDataServiceImpl() {;}

            DetectorDataServiceImpl() {
                setupLocalAliases();
            }

            /**
             * Get the TGeoManager that was setup when loading the detector.
             * This will return null until initialize() is called.
             * @return The current geometry manager or null if not setup.
             */
            TGeoManager* getGeometryManager() {
                return geoManager_;
            }

            /**
             * Get the top DetectorElement, pointing to the "world volume."
             */
            DetectorElement* getTopDetectorElement() {
                return topDE_;
            }

            /**
             * Initialize the ROOT geometry system from the current detector name.
             * This will load the GDML geometry and setup the DetectorElement
             * hierarchy.
             */
            void initialize();

            /**
             * Set the name of the detector to be loaded when initialize() is called.
             * If the detector name is never set then an error will be thrown
             * during initialization.
             * @param detectorName The name of the detector.
             */
            void setDetectorName(std::string detectorName) {
                detectorName_ = detectorName;
            }

            /**
             * Add an alias mapping a detector name to a physical location
             * such as a filesystem path.
             * @param detectorName The name of the detector.
             * @param alias The alias of the detector (i.e. file path).
             */
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
            DetectorElementImpl* topDE_{nullptr};
    };
}

#endif /* DETDESCR_DETECTORSERVICEIMPL_H_ */
