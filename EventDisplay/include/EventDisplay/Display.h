/**
 * @file Display.h
 * @author Josh Hiltbrand, University of Minnesota
 */

#ifndef EVENTDISPLAY_EVENTDISPLAY_H_
#define EVENTDISPLAY_EVENTDISPLAY_H_

#include "TFile.h"
#include "TGButton.h"
#include "TGFrame.h"
#include "TGLViewer.h"
#include "TGTextEntry.h"
#include "TRint.h"
#include "TString.h"
#include "TTree.h"

#include "TEveBrowser.h"
#include "TEveElement.h"
#include "TEveEventManager.h"
#include "TEveManager.h"
#include "TEveViewer.h"

#include "TBranchElement.h"

#include "EventDisplay/EveDetectorGeometry.h"
#include "EventDisplay/EventObjects.h"

#include "Framework/Event.h"
#include "Framework/EventFile.h"

#include <iostream>

namespace eventdisplay {

class Display : public TGMainFrame {
 public:
  /**
   * Constructor
   * Builds window frame and and control panel.
   * Imports geometry from EveDetectorGeometry.
   */
  Display(TEveManager* manager, bool verbose);

  /**
   * Destructor
   * Deletes hanging pointers from constructor and closes the TFile.
   */
  virtual ~Display() {
    the_file_.reset(nullptr);
    delete theDetector_;
    delete eventObjects_;

    delete textBoxClustersCollName_;
    delete textBoxSimThresh_;
    delete textBoxEcalRecHitsCollName_;
    delete textBoxHcalRecHitsCollName_;
    delete textBoxTrackerHitsCollName_;
    delete textBoxEcalScorePlaneBranch_;

    delete manager_;
  }

  /**
   * Opens input file and attempts to obtain the necessary information from it.
   *
   * Attempts to import the event objects from the event tree using the 'Get...'
   * methods below.
   *
   * @param file name of file with events
   * @return true if successfully opened file and found tree named
   * eventTreeName_
   * @return false unable to open file or find tree named eventTreeName_
   */
  bool SetFile(const TString file);

  /**
   * Goes forward one event unless the current event number equals the maximum
   * event number.
   */
  void NextEvent();

  /**
   * Gets ecalRecHits collection name from text box
   */
  void GetECALRecHitsCollInput();

  /**
   * Gets clusters collection name from text box
   */
  void GetClustersCollInput();

  /**
   * Gets trackerHits collection name from text box
   */
  void GetTrackerHitsCollInput();

  /**
   * Gets hcalRecHits collection name from text box
   */
  void GetHCALRecHitsCollInput();

  /**
   * Gets ECAL Sim Particles Branch name from text box
   */
  void GetEcalSimParticlesCollInput();

  /**
   * Sets threshold energy from a SimParticle to be drawn from text box.
   * Re-draws the display.
   * @return bool success check
   */
  bool SetSimThresh();

  /**
   * Colors cluster objects and redraws.
   */
  void ColorClusters();

 private:
  /**
   * Get the text from the input text box
   */
  std::string getText(TGTextEntry* box) const {
    return box->GetText();
  }

 private:
  bool verbose_;  //* verbosity flag

  /// Event bus for reading from input file
  framework::Event the_event_{"display"};

  /// Handle to input file we will be reading
  std::unique_ptr<framework::EventFile> the_file_;

  /// name of ecal clusters collection in event tree
  std::string clustersCollName_ = "ecalClusters"; 
  /// name of ecalRecHits collection in event tree
  std::string ecalRecHitsCollName_ = "EcalRecHits";
  /// name of hcalRecHits collection in event tree
  std::string hcalRecHitsCollName_ = "HcalRecHits";
  /// name of recoil hitss collection in event tree 
  std::string trackerHitsCollName_ = "RecoilSimHits";
  /// name of ecal sim particles collection in
  std::string ecalSimParticlesCollName_ = "EcalScoringPlaneHits";

  EveDetectorGeometry* theDetector_{nullptr};  //* detector geometry instance
  EventObjects* eventObjects_{nullptr};  //* drawing methods for event objects

  TGTextEntry* textBoxClustersCollName_;
  TGTextEntry* textBoxSimThresh_;
  TGTextEntry* textBoxEcalRecHitsCollName_;
  TGTextEntry* textBoxHcalRecHitsCollName_;
  TGTextEntry* textBoxTrackerHitsCollName_;
  TGTextEntry* textBoxEcalScorePlaneBranch_;

  TEveManager* manager_{nullptr};  //* event display manager

  ClassDef(Display, 2);
};
}  // namespace eventdisplay

#endif
