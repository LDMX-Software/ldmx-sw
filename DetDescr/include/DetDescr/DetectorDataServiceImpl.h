/*
 * DetectorServiceImpl.h
 * @brief Class implementing the DetectorDataService
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORSERVICEIMPL_H_
#define DETDESCR_DETECTORSERVICEIMPL_H_

// LDMX
#include "DetDescr/DetectorElementImpl.h"
#include "DetDescr/DetectorDataService.h"

namespace ldmx {

    /**
     * @class DetectorDataServiceImpl
     * @brief Implements the DetectorDataService for accessing detector data at runtime
     */
    class DetectorDataServiceImpl : public DetectorDataService {

        public:

            /**
             * Map of detector names to their physical resources on disk.
             */
            typedef std::map<std::string, std::string> DetectorAliasMap;

            /**
             * Class destructor.
             *
             * @note
             * Deletes the DetectorElement hierarchy and the ROOT geometry manager.
             */
            virtual ~DetectorDataServiceImpl() {
                delete topDE_;
                delete geoManager_;
            }

            /**
             * Class constructor.
             *
             * @note
             * Builds the cache of names to local aliases, which are file system paths of
             * full detector GDML files.
             */
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
                //std::cout << "[ DetectorDataServiceImpl ] : Added alias " << detectorName << " => " << alias << std::endl;
            }

        private:

            /**
             * Build a cache of detector aliases from names to their locations on
             * disk, using the LDMXSW_DIR environment variable to locate the main
             * detector data directory in the installation area.
             */
            void setupLocalAliases();

        private:

            /** Name of the current detector. */
            std::string detectorName_;

            /** Map of names to detector locations (GDML files). */
            DetectorAliasMap aliasMap_;

            /** The currently active geometry manager. */
            TGeoManager* geoManager_{nullptr};

            /** The top DetectorElement providing access to the hierarchy. */
            DetectorElementImpl* topDE_{nullptr};
    };

    /**
     * @class GlobalPositionCacheBuilder
     * @brief Caches the global positions of every DetectorElement in the hierarchy
     */
    class GlobalPositionCacheBuilder : public DetectorElementVisitor {

        public:

            /**
             * Class constructor.
             * @param nav The TGeoNavigator for navigating the detector geometry.
             */
            GlobalPositionCacheBuilder(TGeoNavigator* nav) : nav_(nav) {;}

            /**
             * Visit the given DetectorElement and set its global position.
             * @param de The DetectorElement to visit.
             */
            void visit(DetectorElement* de);

        private:

            /** The geometry navigator. */
            TGeoNavigator* nav_;
    };
}

#endif /* DETDESCR_DETECTORSERVICEIMPL_H_ */
