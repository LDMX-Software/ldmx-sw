/*
 * DetectorElement.h
 * @brief An identifiable component in the detector hierarchy such as a layer in a subdetector
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_DETECTORELEMENT_H_
#define DETDESCR_DETECTORELEMENT_H_

#include "TGeoNode.h"

#include "DetDescr/DetectorID.h"

#include <vector>

namespace ldmx {

    class DetectorElement {

        public:

            typedef std::vector<DetectorElement*> DetectorElementVector;

            virtual ~DetectorElement() {;}

            virtual TGeoNode* getSupport() = 0;

            virtual bool hasSupport() = 0;

            virtual const DetectorElementVector& getChildren() = 0;

            virtual DetectorElement* getParent() = 0;

            virtual TGeoVolume* getVolume() = 0;

            virtual DetectorID::RawValue getID() = 0;

            virtual DetectorID* getDetectorID() = 0;

            virtual const std::string& getName() = 0;
    };
}

#endif /* DETDESCR_DETECTORELEMENT_H_ */
