/*
 * @file DetectorDataSerivce.h
 * @brief Class providing a service for loading detector data at runtime
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORDATASERIVCE_H_
#define DETDESCR_DETECTORDATASERIVCE_H_

#include "TGeoManager.h"

#include "DetDescr/DetectorElement.h"

#include <vector>

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
             * Map of detector names to their files on disk.
             */
            typedef std::map<std::string, std::string> DetectorAliasMap;

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
            
            /**
             * Get a DetectorElement by its ID.
             * @return The DetectorElement with the id or null if none exists.
             */
            virtual DetectorElement* getDetectorElement(int id) = 0;
           
            /**
             * Get a DetectorElement by its assigned node.
             * @return The DetectorElement with the matching node or the top DetectorElement if none exists.
             *
             * @note If the node is not explicitly assigned to a DetectorElement,
             * the geometry hierarchy will be searched upwards for the containing
             * DetectorElement.  In case none is found, the top DetectorElement
             * will be returned.
             */
            virtual DetectorElement* findDetectorElement(TGeoNode* node) = 0;

            /**
             * Locate a DetectorElement leaf from a global position.
             * @return The DetectorElement containing the position.
             */            
            virtual DetectorElement* locateDetectorElement(std::vector<double>& globalPosition) = 0;

            /**
             * Get the detector aliases mapping names to files.
             * @return The detector alias map.
             */
            virtual const DetectorAliasMap& getDetectorAliases() = 0;
    };
}

#endif /* DETDESCR_DETECTORDATASERIVCE_H_ */
