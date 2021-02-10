/**
 * @file EveDetectorGeometry.h
 * @author Tom Eichlersmith, University of Minnesota
 * @author Josh Hiltbrand, University of Minnesota
 * @brief Header file for EveDetectorGeometry Class
 */

#ifndef EVENTDISPLAY_EVEDETECTORGEOMETRY_H_
#define EVENTDISPLAY_EVEDETECTORGEOMETRY_H_

#include "DetDescr/HcalID.h"  //for HcalSection enum
#include "EventDisplay/DetectorGeometry.h"
#include "EventDisplay/EveShapeDrawer.h"
#include "TEveElement.h"

namespace eventdisplay {

/**
 * @class EveDetectorGeometry
 * @brief Class that constructs the detector components for the event display
 */
class EveDetectorGeometry {
 public:
  /**
   * Constructor
   * Builds and draws all of the detector elements.
   */
  EveDetectorGeometry();

  /**
   * Destructor
   * Cleanup leftover pointers.
   */
  ~EveDetectorGeometry() {
    delete hcal_;
    delete sidehcal_;
    delete ecal_;
    delete recoilTracker_;
    delete detector_;
  }

  /**
   * Draw the elements of the ECAL
   */
  void drawECAL();

  /**
   * Draw the elements of the HCAL
   */
  void drawHCAL();

  /**
   * Draw the elements of the Recoil Tracker
   */
  void drawRecoilTracker();

  /**
   * Access ECAL Eve Element
   */
  TEveElement* getECAL() { return ecal_; }

  /**
   * Access HCAL Eve Element
   */
  TEveElement* getHCAL() { return hcal_; }

  /**
   * Access Recoil Tracker Eve Element
   */
  TEveElement* getRecoilTracker() { return recoilTracker_; }

  /**
   * Access Entire Detector Eve Element
   */
  TEveElement* getDetector() { return detector_; }

 private:
  TEveElement* hcal_;           //* HCAL Eve Element
  TEveElement* sidehcal_;       //* Side HCAL Eve Element
  TEveElement* ecal_;           //* ECAL Eve Element
  TEveElement* recoilTracker_;  //* Recoil Tracker Eve Element
  TEveElement* detector_;       //* Entire Detector Eve Element
};
}  // namespace eventdisplay

#endif
