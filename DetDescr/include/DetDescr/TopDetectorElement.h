/*
 * TopDetectorElement.h
 * @brief Top DetectorElement in the hierarchy containing the subdetectors
 * @author JeremyMcCormick, SLAC
 */

#ifndef DETDESCR_TOPDETECTORELEMENT_H_
#define DETDESCR_TOPDETECTORELEMENT_H_

#include "DetDescr/DetectorElementImpl.h"

namespace ldmx {

    class TopDetectorElement : public DetectorElementImpl {

        public:

            TopDetectorElement(TGeoNode* support);

            //EcalDetectorElement* getEcal();

            //HcalDetectorElement* getHcal();

            //TaggerDetectorElement* getTagger();

            //RecoilTrackerDetectorElement* getRecoilTracker();
    };
}


#endif /* DETDESCR_TOPDETECTORELEMENT_H_ */
