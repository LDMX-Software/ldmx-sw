/*
 * DetectorDataSerivce.h
 * @brief Class providing a service for loading detector data at runtime
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORDATASERIVCE_H_
#define DETDESCR_DETECTORDATASERIVCE_H_

#include "TGeoManager.h"

#include "DetDescr/DetectorElement.h"

namespace ldmx {

    /**
     * @class DetectorDataService
     * @brief Provides an API for accessing detector data at runtime through the DetectorElement interface
     *
     * @note
     * Access to the backing ROOT geometry manager is also provided.
     */
    class DetectorDataService {

        public:

            /**
             * Class destructor.
             */
            virtual ~DetectorDataService() {;}

            /**
             * Get the backing TGeoManager with the physical geometry.
             * @return The backing TGeoManager.
             */
            virtual TGeoManager* getGeometryManager() = 0;

            /**
             * Get the top DetectorElement in the hierarchy.
             * @return The top top DetectorElement in the hierarchy.
             */
            virtual DetectorElement* getTopDetectorElement() = 0;

            /**
             * Get the name of the current detector.
             * @return The name of the current detector.
             */
            virtual const std::string& getDetectorName() = 0;

            /**
             * Initialize the geometry.
             */
            virtual void initialize() = 0;
    };
}

#endif /* DETDESCR_DETECTORDATASERIVCE_H_ */
