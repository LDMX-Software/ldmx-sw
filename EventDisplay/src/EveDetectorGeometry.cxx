#include "EventDisplay/EveDetectorGeometry.h"

namespace ldmx {

    EveDetectorGeometry::EveDetectorGeometry() {

        hcal_ = new TEveElementList("HCAL");
        sidehcal_ = new TEveElementList("Side HCAL");
        ecal_ = new TEveElementList("ECAL");
        recoilTracker_ = new TEveElementList("Recoil Tracker");
        detector_ = new TEveElementList("LDMX Detector");

        shapeDrawer_ = new EveShapeDrawer();

        drawECAL();
        drawHCAL();
        drawRecoilTracker();

    }

    void EveDetectorGeometry::drawECAL() {


        for (int col = 0; col < 7; ++col) {
    
            TString colName;
            colName.Form("Tower %d", col);
            TEveGeoShape* hexCol = shapeDrawer_->drawHexPrism(
                    DETECTOR_GEOMETRY.getHexPrism( col ),
                    0, 0, 0, 
                    kBlue, 90, colName);

            ecal_->AddElement(hexCol);
        }
    
        detector_->AddElement(ecal_);
    }

    void EveDetectorGeometry::drawHCAL() {
    
        TEveGeoShape* backHcal = shapeDrawer_->drawRectPrism(
                DETECTOR_GEOMETRY.getBoundingBox( HcalSection::BACK ),
                0, 0, 0, kCyan, 90, "Back HCal"); 
        hcal_->AddElement(backHcal);
    
        TEveGeoShape* sideTopHcal = shapeDrawer_->drawRectPrism(
                DETECTOR_GEOMETRY.getBoundingBox( HcalSection::TOP ),
                0, 0, 0, kCyan, 90, "Module 1");

        sidehcal_->AddElement(sideTopHcal);
    
        TEveGeoShape* sideBottomHcal = shapeDrawer_->drawRectPrism(
                DETECTOR_GEOMETRY.getBoundingBox( HcalSection::BOTTOM ),
                0, 0, 0, kCyan, 90, "Module 4");

        sidehcal_->AddElement(sideBottomHcal);
    
        TEveGeoShape* sideLeftHcal = shapeDrawer_->drawRectPrism(
                DETECTOR_GEOMETRY.getBoundingBox( HcalSection::LEFT ),
                0, 0, 0, kCyan, 90, "Module 2");

        sidehcal_->AddElement(sideLeftHcal);
    
        TEveGeoShape* sideRightHcal = shapeDrawer_->drawRectPrism(
                DETECTOR_GEOMETRY.getBoundingBox( HcalSection::RIGHT ),
                0, 0, 0, kCyan, 90, "Module 3");

        sidehcal_->AddElement(sideRightHcal);
        hcal_->AddElement(sidehcal_);
    
        detector_->AddElement(hcal_);
    }
    
    void EveDetectorGeometry::drawRecoilTracker() {
    
        for (int layerID = 1; layerID < 9; layerID++ ) {
    
            TString name;
            name.Form("Stereo_%d", layerID );
    
            TEveGeoShape *layer = shapeDrawer_->drawRectPrism(
                    DETECTOR_GEOMETRY.getBoundingBox( layerID , 0 ),
                    0, 0, DETECTOR_GEOMETRY.getRotAngle( layerID , 0 )*180/M_PI, 
                    kRed-10, 90, name );
    
            recoilTracker_->AddElement(layer);
        }
    
        for (int layerID = 9; layerID < 11; layerID++ ) {
            for (int moduleID = 0; moduleID < 10; moduleID++ ) {
    
                TString name;
                name.Form("Mono_%d_%d" , layerID , moduleID );

         
                TEveGeoShape *layer = shapeDrawer_->drawRectPrism(
                        DETECTOR_GEOMETRY.getBoundingBox( layerID , moduleID ),
                        0, 0, DETECTOR_GEOMETRY.getRotAngle( layerID , moduleID )*180/M_PI, 
                        kRed-10, 90, name );
        
                recoilTracker_->AddElement(layer);
            }
        }
    
        detector_->AddElement(recoilTracker_);

        return;
    }
}
