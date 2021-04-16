/**
 * @file EveDetectorGeometry.cxx
 * @author Tom Eichlersmith, University of Minnesota
 * @author Josh Hiltbrand, University of Minnesota
 * @brief Implementation file for EveDetectorGeometry Class
 */

#include "EventDisplay/EveDetectorGeometry.h"

namespace eventdisplay {

EveDetectorGeometry::EveDetectorGeometry() {
  hcal_ = new TEveElementList("HCAL");
  sidehcal_ = new TEveElementList("Side HCAL");
  ecal_ = new TEveElementList("ECAL");
  recoilTracker_ = new TEveElementList("Recoil Tracker");
  detector_ = new TEveElementList("LDMX Detector");

  drawECAL();
  drawHCAL();
  drawRecoilTracker();
}

void EveDetectorGeometry::drawECAL() {
  for (int col = 0; col < 7; ++col) {
    TString colName;
    colName.Form("Tower %d", col);
    TEveGeoShape* hexCol = EveShapeDrawer::getInstance().drawHexPrism(
        DetectorGeometry::getInstance().getHexTower(col), 0, 0, 0, kBlue, 90,
        colName);

    ecal_->AddElement(hexCol);
  }

  detector_->AddElement(ecal_);
}

void EveDetectorGeometry::drawHCAL() {
  TEveGeoShape* backHcal = EveShapeDrawer::getInstance().drawRectPrism(
      DetectorGeometry::getInstance().getBoundingBox(ldmx::HcalID::HcalSection::BACK),
      0, 0, 0, kCyan, 90, "Back HCal");
  hcal_->AddElement(backHcal);

  TEveGeoShape* sideTopHcal = EveShapeDrawer::getInstance().drawRectPrism(
      DetectorGeometry::getInstance().getBoundingBox(ldmx::HcalID::HcalSection::TOP),
      0, 0, 0, kCyan, 90, "Module 1");

  sidehcal_->AddElement(sideTopHcal);

  TEveGeoShape* sideBottomHcal = EveShapeDrawer::getInstance().drawRectPrism(
      DetectorGeometry::getInstance().getBoundingBox(
          ldmx::HcalID::HcalSection::BOTTOM),
      0, 0, 0, kCyan, 90, "Module 4");

  sidehcal_->AddElement(sideBottomHcal);

  TEveGeoShape* sideLeftHcal = EveShapeDrawer::getInstance().drawRectPrism(
      DetectorGeometry::getInstance().getBoundingBox(ldmx::HcalID::HcalSection::LEFT),
      0, 0, 0, kCyan, 90, "Module 2");

  sidehcal_->AddElement(sideLeftHcal);

  TEveGeoShape* sideRightHcal = EveShapeDrawer::getInstance().drawRectPrism(
      DetectorGeometry::getInstance().getBoundingBox(
          ldmx::HcalID::HcalSection::RIGHT),
      0, 0, 0, kCyan, 90, "Module 3");

  sidehcal_->AddElement(sideRightHcal);
  hcal_->AddElement(sidehcal_);

  detector_->AddElement(hcal_);
}

void EveDetectorGeometry::drawRecoilTracker() {
  for (int layerID = 1; layerID < 9; layerID++) {
    TString name;
    name.Form("Stereo_%d", layerID);

    TEveGeoShape* layer = EveShapeDrawer::getInstance().drawRectPrism(
        DetectorGeometry::getInstance().getBoundingBox(layerID, 0), 0, 0,
        DetectorGeometry::getInstance().getRotAngle(layerID, 0) * 180 / M_PI,
        kRed - 10, 90, name);

    recoilTracker_->AddElement(layer);
  }

  for (int layerID = 9; layerID < 11; layerID++) {
    for (int moduleID = 0; moduleID < 10; moduleID++) {
      TString name;
      name.Form("Mono_%d_%d", layerID, moduleID);

      TEveGeoShape* layer = EveShapeDrawer::getInstance().drawRectPrism(
          DetectorGeometry::getInstance().getBoundingBox(layerID, moduleID), 0,
          0,
          DetectorGeometry::getInstance().getRotAngle(layerID, moduleID) * 180 /
              M_PI,
          kRed - 10, 90, name);

      recoilTracker_->AddElement(layer);
    }
  }

  detector_->AddElement(recoilTracker_);

  return;
}
}  // namespace eventdisplay
