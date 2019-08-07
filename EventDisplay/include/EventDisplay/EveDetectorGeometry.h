#ifndef EVENTDISPLAY_DETECTORGEOMETRY_H_
#define EVENTDISPLAY_DETECTORGEOMETRY_H_

#include "TEveElement.h"
#include "EventDisplay/EveShapeDrawer.h"
#include "DetDescr/DetectorGeometry.h"
#include "DetDescr/HcalID.h" //for HcalSection enum

namespace ldmx {

    class EveDetectorGeometry {

        public:

            EveDetectorGeometry();

            ~EveDetectorGeometry() {

                delete hcal_;
                delete sidehcal_;
                delete ecal_;
                delete recoilTracker_;
                delete detector_;
                delete shapeDrawer_;
            }

            void drawECAL();

            void drawHCAL();

            void drawRecoilTracker();

            TEveElement* getECAL() { return ecal_; }

            TEveElement* getHCAL() { return hcal_; }

            TEveElement* getRecoilTracker() { return recoilTracker_; }

            TEveElement* getDetector() { return detector_; }

        private:

            TEveElement* hcal_;
            TEveElement* sidehcal_;
            TEveElement* ecal_;
            TEveElement* recoilTracker_;
            TEveElement* detector_;

            EveShapeDrawer* shapeDrawer_;
    };
}

#endif
