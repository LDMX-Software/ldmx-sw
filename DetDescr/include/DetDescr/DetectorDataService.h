/*
 * DetectorDataSerivce.h
 * @brief Service for loading detector data at runtime
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORDATASERIVCE_H_
#define DETDESCR_DETECTORDATASERIVCE_H_

// ROOT
#include "TGeoManager.h"

namespace ldmx {

    class DetectorDataService {

        public:

            virtual ~DetectorDataService() {;}

            virtual TGeoManager* getGeometryManager() = 0;

            virtual DetectorElement* getTopDetectorElement() = 0;

            virtual void initialize() = 0;
    };
}




#endif /* DETDESCR_DETECTORDATASERIVCE_H_ */
