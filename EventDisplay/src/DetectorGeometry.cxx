#include "EventDisplay/DetectorGeometry.h"

namespace ldmx {

    DetectorGeometry::DetectorGeometry() {

        hcal_ = new TEveElementList("HCAL");
        sidehcal_ = new TEveElementList("Side HCAL");
        ecal_ = new TEveElementList("ECAL");
        recoilTracker_ = new TEveElementList("Recoil Tracker");
        detector_ = new TEveElementList("LDMX Detector");

        drawECAL();
        drawHCAL();
        drawRecoilTracker();

    }

    void DetectorGeometry::drawECAL() {
    
        static const std::vector<double> xPos = {0, 0, 0, 170*sqrt(3)/2, -170*sqrt(3)/2, -170*sqrt(3)/2, 170*sqrt(3)/2};
        static const std::vector<double> yPos = {0, 170, -170, 85, 85, -85, -85};
        static const std::vector<double> zPos = {-140, 150};
    
        for (int col = 0; col < xPos.size(); ++col) {
    
            char colName[50];
            sprintf(colName, "Tower %d", col);
            TEveStraightLineSet* hexCol = shapeDrawer_->drawHexColumn(xPos[col], yPos[col], zPos[0]+ECAL_Z_OFFSET, zPos[1]+ECAL_Z_OFFSET, 170, kBlue, colName);
            ecal_->AddElement(hexCol);
        }
    
        detector_->AddElement(ecal_);
    }
    
    void DetectorGeometry::drawHCAL() {
    
        TEveBox *backHcal = shapeDrawer_->drawBox(0, 0, HCAL_Z_OFFSET, hcal_x_width, hcal_y_width, HCAL_Z_OFFSET+hcal_z_length, 0, kCyan, 100, "Back HCAL");
        hcal_->AddElement(backHcal);
    
        TEveBox *sideTopHcal = shapeDrawer_->drawBox(hcal_y_width/4-hcal_ecal_xy/4, -hcal_y_width/4-hcal_ecal_xy/4, HCAL_Z_OFFSET-hcal_side_z, (hcal_y_width+hcal_ecal_xy)/2, (hcal_y_width-hcal_ecal_xy)/2, HCAL_Z_OFFSET, 0, kCyan, 100, "Module 1");
        sidehcal_->AddElement(sideTopHcal);
    
        TEveBox *sideBottomHcal = shapeDrawer_->drawBox(hcal_y_width/4+hcal_ecal_xy/4, hcal_y_width/4-hcal_ecal_xy/4, HCAL_Z_OFFSET-hcal_side_z, (hcal_y_width-hcal_ecal_xy)/2, (hcal_y_width+hcal_ecal_xy)/2, HCAL_Z_OFFSET, 0, kCyan, 100, "Module 4");
        sidehcal_->AddElement(sideBottomHcal);
    
        TEveBox *sideLeftHcal = shapeDrawer_->drawBox(-hcal_y_width/4+hcal_ecal_xy/4, hcal_y_width/4+hcal_ecal_xy/4, HCAL_Z_OFFSET-hcal_side_z, (hcal_y_width+hcal_ecal_xy)/2, (hcal_y_width-hcal_ecal_xy)/2, HCAL_Z_OFFSET, 0, kCyan, 100, "Module 2");
        sidehcal_->AddElement(sideLeftHcal);
    
        TEveBox *sideRightHcal = shapeDrawer_->drawBox(-hcal_y_width/4-hcal_ecal_xy/4, -hcal_y_width/4+hcal_ecal_xy/4, HCAL_Z_OFFSET-hcal_side_z, (hcal_y_width-hcal_ecal_xy)/2, (hcal_y_width+hcal_ecal_xy)/2, HCAL_Z_OFFSET, 0, kCyan, 100, "Module 3");
        sidehcal_->AddElement(sideRightHcal);
        hcal_->AddElement(sidehcal_);
    
        //turn off HCAL by default
        hcal_->SetRnrSelf(0);
    
        detector_->AddElement(hcal_);
    }
    
    void DetectorGeometry::drawRecoilTracker() {
    
        // In mm
        static const double stereo_x_width = 40.34;
        static const double stereo_y_width = 100;
        static const double mono_x_width = 50;
        static const double mono_y_width = 80;
    
        const std::vector<double> xPos = {-2*mono_x_width, -mono_x_width, 0, mono_x_width, 2*mono_x_width};
        const std::vector<double> yPos = {-mono_y_width/2, mono_y_width/2};
        const std::vector<double> zPos = {7.5, 22.5, 37.5, 52.5, 90.0, 180.0};
    
        for (int j = 0; j < 4; ++j) {
    
            char nfront[50];
            sprintf(nfront, "Stereo%d_front", j+1);
    
            char nback[50];
            sprintf(nback, "Stereo%d_back", j+1);
    
            TEveBox *front = shapeDrawer_->drawBox(0, 0, zPos[j]-STEREO_SEP, stereo_x_width, stereo_y_width, zPos[j]-STEREO_SEP+RECOIL_SENSOR_THICKNESS, 0, kRed-10, 100, nfront);
    
            if (j % 2 == 0) { // Alternate angle for back layer of a stereo pair.
                TEveBox *back = shapeDrawer_->drawBox(0, 0, zPos[j]+STEREO_SEP, stereo_x_width, stereo_y_width, zPos[j]+STEREO_SEP+RECOIL_SENSOR_THICKNESS, STEREO_ANGLE, kRed-10, 100, nback);
                recoilTracker_->AddElement(back);
            } else {
                TEveBox *back = shapeDrawer_->drawBox(0, 0, zPos[j]+STEREO_SEP, stereo_x_width, stereo_y_width, zPos[j]+STEREO_SEP+RECOIL_SENSOR_THICKNESS, -STEREO_ANGLE, kRed-10, 100, nback);
                recoilTracker_->AddElement(back);
            }
    
            recoilTracker_->AddElement(front);
        }
    
        int module1 = 1;
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 2; ++y) {
    
                char name[50];
                sprintf(name,"Mono1_%d",module1);
                ++module1;
    
                if (x % 2 != 0) { // Alternate mono layer z by defined separation.
                    TEveBox *front = shapeDrawer_->drawBox(xPos[x], yPos[y], zPos[4]-MONO_SEP, mono_x_width, mono_y_width, zPos[4]-MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kRed-10, 100, name);
                    recoilTracker_->AddElement(front);
                } else {
                    TEveBox *back = shapeDrawer_->drawBox(xPos[x], yPos[y], zPos[4]+MONO_SEP, mono_x_width, mono_y_width, zPos[4]+MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kRed-10, 100, name);
                    recoilTracker_->AddElement(back);
                }
            }
        }
    
        int module2 = 1;
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 2; ++y) {
    
                char name[50];
                sprintf(name,"Mono2_%d",module2);
                module2++;
    
                if (x % 2 != 0) { // Alternate mono layer z by defined separation.
                    TEveBox *front = shapeDrawer_->drawBox(xPos[x], yPos[y], zPos[5]-MONO_SEP, mono_x_width, mono_y_width, zPos[5]-MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kRed-10, 100, name);
                    recoilTracker_->AddElement(front);
                } else {
                    TEveBox *back = shapeDrawer_->drawBox(xPos[x], yPos[y], zPos[5]+MONO_SEP, mono_x_width, mono_y_width, zPos[5]+MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kRed-10, 100, name);
                    recoilTracker_->AddElement(back);
                }
            }
        }
    
        detector_->AddElement(recoilTracker_);
    }
}
