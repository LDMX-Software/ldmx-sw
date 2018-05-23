#ifndef EVENTDISPLAY_DETECTORGEOMETRY_H_
#define EVENTDISPLAY_DETECTORGEOMETRY_H_

#include "TEveElement.h"
#include "EventDisplay/EveShapeDrawer.h"

// All lengths are in mm
static const double ECAL_Z_OFFSET = 214.5+290.0/2; //First number is distance from target to ECAL front face, second is half ECAL extent in z
static const double HCAL_Z_OFFSET = 504.5;
static const double HCAL_ZHIT_OFFSET = 4.5;
static const double RECOIL_SENSOR_THICKNESS = 0.52;
static const double STEREO_SEP = 3;
static const double MONO_SEP = 1;
// In mm
static const double stereo_strip_length = 98; // 2 mm deadspace
static const double mono_strip_length = 78; // 2 mm deadspace

static const double layerZPos[] = {-137.2, -134.3, -127.95, -123.55, -115.7, -109.8, -100.7, -94.3, -85.2, -78.8, -69.7, -63.3, -54.2, -47.8, -38.7, -32.3, -23.2, -16.8, -7.7, -1.3, 7.8, 14.2, 23.3, 29.7, 42.3, 52.2, 64.8, 74.7, 87.3, 97.2, 109.8, 119.7, 132.3, 142.2};

static const double hcal_x_width = 3100.0;
static const double hcal_y_width = 3100.0;
static const double hcal_z_length = 3000.0;
static const double hcal_side_z = 290.0;
static const double hcal_top_x = 525.0;
static const double hcal_ecal_xy = 512.0;

// In radians
static const double STEREO_ANGLE = 0.1; 

namespace ldmx {

    class DetectorGeometry {

        public:

            DetectorGeometry();

            ~DetectorGeometry() {}

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
