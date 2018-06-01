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
    
        for (int col = 0; col < towerXPos.size(); ++col) {
            
    
            char colName[50];
            sprintf(colName, "Tower %d", col);
            TEveStraightLineSet* hexCol = shapeDrawer_->drawHexColumn(towerXPos[col], towerYPos[col], ecal_front_z, ecal_front_z+ecal_z_length, 170, kBlue, colName);
            ecal_->AddElement(hexCol);
        }
    
        detector_->AddElement(ecal_);
    }
    
    void DetectorGeometry::drawHCAL() {
    
        TEveBox *backHcal = shapeDrawer_->drawBox(0, 0, hcal_front_z, hcal_x_width, hcal_y_width, hcal_front_z+hcal_z_length, 0, kCyan, 100, "Back HCAL");
        hcal_->AddElement(backHcal);
    
        TEveBox *sideTopHcal = shapeDrawer_->drawBox(-hcal_y_width/4+hcal_ecal_xy/4, -hcal_y_width/4-hcal_ecal_xy/4, hcal_front_z-hcal_side_z, (hcal_y_width+hcal_ecal_xy)/2, (hcal_y_width-hcal_ecal_xy)/2, hcal_front_z, 0, kCyan, 100, "Module 1");
        sidehcal_->AddElement(sideTopHcal);
    
        TEveBox *sideBottomHcal = shapeDrawer_->drawBox(-hcal_y_width/4-hcal_ecal_xy/4, hcal_y_width/4-hcal_ecal_xy/4, hcal_front_z-hcal_side_z, (hcal_y_width-hcal_ecal_xy)/2, (hcal_y_width+hcal_ecal_xy)/2, hcal_front_z, 0, kCyan, 100, "Module 4");
        sidehcal_->AddElement(sideBottomHcal);
    
        TEveBox *sideLeftHcal = shapeDrawer_->drawBox(hcal_y_width/4-hcal_ecal_xy/4, hcal_y_width/4+hcal_ecal_xy/4, hcal_front_z-hcal_side_z, (hcal_y_width+hcal_ecal_xy)/2, (hcal_y_width-hcal_ecal_xy)/2, hcal_front_z, 0, kCyan, 100, "Module 2");
        sidehcal_->AddElement(sideLeftHcal);
    
        TEveBox *sideRightHcal = shapeDrawer_->drawBox(hcal_y_width/4+hcal_ecal_xy/4, -hcal_y_width/4+hcal_ecal_xy/4, hcal_front_z-hcal_side_z, (hcal_y_width-hcal_ecal_xy)/2, (hcal_y_width+hcal_ecal_xy)/2, hcal_front_z, 0, kCyan, 100, "Module 3");
        sidehcal_->AddElement(sideRightHcal);
        hcal_->AddElement(sidehcal_);
    
        detector_->AddElement(hcal_);
    }
    
    void DetectorGeometry::drawRecoilTracker() {
    
        for (int j = 0; j < 4; ++j) {
    
            char nfront[50];
            sprintf(nfront, "Stereo%d_front", j+1);
    
            char nback[50];
            sprintf(nback, "Stereo%d_back", j+1);
    
            TEveBox *front = shapeDrawer_->drawBox(0, 0, recoilLayerZPos[j]-stereo_sep, stereo_x_width, stereo_y_width, recoilLayerZPos[j]-stereo_sep+recoil_sensor_thick, 0, kRed-10, 100, nfront);
    
            if (j % 2 == 0) { // Alternate angle for back layer of a stereo pair.
                TEveBox *back = shapeDrawer_->drawBox(0, 0, recoilLayerZPos[j]+stereo_sep, stereo_x_width, stereo_y_width, recoilLayerZPos[j]+stereo_sep+recoil_sensor_thick, stereo_angle, kRed-10, 100, nback);
                recoilTracker_->AddElement(back);
            } else {
                TEveBox *back = shapeDrawer_->drawBox(0, 0, recoilLayerZPos[j]+stereo_sep, stereo_x_width, stereo_y_width, recoilLayerZPos[j]+stereo_sep+recoil_sensor_thick, -stereo_angle, kRed-10, 100, nback);
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
                    TEveBox *front = shapeDrawer_->drawBox(monoSensorXPos[x], monoSensorYPos[y], recoilLayerZPos[4]-mono_sep, mono_x_width, mono_y_width, recoilLayerZPos[4]-mono_sep+recoil_sensor_thick, 0, kRed-10, 100, name);
                    recoilTracker_->AddElement(front);
                } else {
                    TEveBox *back = shapeDrawer_->drawBox(monoSensorXPos[x], monoSensorYPos[y], recoilLayerZPos[4]+mono_sep, mono_x_width, mono_y_width, recoilLayerZPos[4]+mono_sep+recoil_sensor_thick, 0, kRed-10, 100, name);
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
                    TEveBox *front = shapeDrawer_->drawBox(monoSensorXPos[x], monoSensorYPos[y], recoilLayerZPos[5]-mono_sep, mono_x_width, mono_y_width, recoilLayerZPos[5]-mono_sep+recoil_sensor_thick, 0, kRed-10, 100, name);
                    recoilTracker_->AddElement(front);
                } else {
                    TEveBox *back = shapeDrawer_->drawBox(monoSensorXPos[x], monoSensorYPos[y], recoilLayerZPos[5]+mono_sep, mono_x_width, mono_y_width, recoilLayerZPos[5]+mono_sep+recoil_sensor_thick, 0, kRed-10, 100, name);
                    recoilTracker_->AddElement(back);
                }
            }
        }
    
        detector_->AddElement(recoilTracker_);
    }
}
