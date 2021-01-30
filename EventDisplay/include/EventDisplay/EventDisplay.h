/**
 * @file EventDisplay.h
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

#include <iostream>

namespace ldmx {

class EventDisplay : public TGMainFrame {
 public:
  /**
   * Constructor
   * Builds window frame and and control panel.
   * Imports geometry from EveDetectorGeometry.
   */
  EventDisplay(TEveManager* manager, bool verbose);

  /**
   * Destructor
   * Deletes hanging pointers from constructor and closes the TFile.
   */
  ~EventDisplay() {
    file_->Close();
    delete file_;
    delete tree_;
    delete theDetector_;
    delete eventObjects_;

    delete textBoxGotoEvent_;
    delete textBoxClustersCollName_;
    delete textBoxSimThresh_;
    delete textBoxEventTreeName_;
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
   * Goes back one event unless the current event number is not positive.
   *
   * @sa GotoEvent(int)
   */
  void PreviousEvent();

  /**
   * Goes forward one event unless the current event number equals the maximum
   * event number.
   *
   * @sa GotoEvent(int)
   */
  void NextEvent();

  /**
   * Attempts to get branch from event tree and set the address to be
   * the input collection.
   *
   * @param branchName name of branch containing collection
   * @param collection vector that will be used in event display
   * @return bool successful check
   */
  template <typename T>
  bool GetCollection(TString branchName, std::vector<T>& collection) {
    if (not branchName.Contains("_")) branchName += "*";
    TBranchElement* br =
        dynamic_cast<TBranchElement*>(tree_->GetBranch(branchName));
    if (br) {
      br->SetObject(&collection);
      if (verbose_) {
        std::cout << "[ EventDisplay ] : Collection retrieved from branch \""
                  << branchName << "\"" << std::endl;
      }
      return true;
    } else {
      std::cout << "[ EventDisplay ] : No branch with name \"" << branchName
                << "\"" << std::endl;
      return false;
    }
  }

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
   * Goes to the input event index if it is not outside the bounds.
   *
   * Destroys the elements from previous event and re-initializes the
   * eventObjects_ instance. Draws all objects that were able to be found.
   *
   * @param event index for the event we want to go to.
   * @return bool success check
   */
  bool GotoEvent(int event);

  /**
   * Takes input of event index from text box.
   */
  bool GotoEvent();

  /**
   * Sets threshold energy from a SimParticle to be drawn from text box.
   * Re-draws the display.
   * @return bool success check
   */
  bool SetSimThresh();

  /**
   * Sets event tree from input text box.
   *
   * @return bool success check
   */
  bool SetEventTree();

  /**
   * Colors cluster objects and redraws.
   */
  void ColorClusters();

 private:
  bool verbose_;  //* verbosity flag

  TFile* file_;  //* Event file
  TTree* tree_;  //* Event tree

  std::vector<EcalHit> ecalRecHits_;       //* current ecalRecHits collection
  std::vector<HcalHit> hcalRecHits_;       //* current hcalRecHits collection
  std::vector<SimTrackerHit> recoilHits_;  //* curent recoil hits collection
  std::vector<EcalCluster> ecalClusters_;  //* current ecal clusters collection
  std::vector<SimTrackerHit>
      ecalSimParticles_;  //* current ecal sim particles collection

  bool foundECALRecHits_ =
      false;  //* flag check if ecalRecHits collection has been found
  bool foundHCALRecHits_ =
      false;  //* flag check if hcalRecHits collection has been found
  bool foundClusters_ =
      false;  //* flag check if clusters collection has been found
  bool foundTrackerHits_ =
      false;  //* flag check if tracker hits collection has been found
  bool foundEcalSPHits_ =
      false;  //* flag check if ecal sim particles collection has been found

  int eventNum_ = -1;  //* current event number
  int eventNumMax_;    ///* maximum event index for the current tree

  TString clustersCollName_ =
      "ecalClusters_recon";  //* name of ecal clusters collection in event tree
  TString ecalRecHitsCollName_ =
      "EcalRecHits_digi";  //* name of ecalRecHits collection in event tree
  TString hcalRecHitsCollName_ =
      "HcalRecHits_digi";  //* name of hcalRecHits collection in event tree
  TString trackerHitsCollName_ =
      "RecoilSimHits_sim";  //* name of recoil hitss collection in event tree
  TString ecalSimParticlesCollName_ =
      "EcalScoringPlaneHits_sim";  //* name of ecal sim particles collection in
                                   // event tree
  TString eventTreeName_ = "LDMX_Events";  //* name of event tree

  EveDetectorGeometry* theDetector_{nullptr};  //* detector geometry instance
  EventObjects* eventObjects_{nullptr};  //* drawing methods for event objects

  TGTextEntry* textBoxGotoEvent_;
  TGTextEntry* textBoxClustersCollName_;
  TGTextEntry* textBoxSimThresh_;
  TGTextEntry* textBoxEventTreeName_;
  TGTextEntry* textBoxEcalRecHitsCollName_;
  TGTextEntry* textBoxHcalRecHitsCollName_;
  TGTextEntry* textBoxTrackerHitsCollName_;
  TGTextEntry* textBoxEcalScorePlaneBranch_;

  TEveManager* manager_{nullptr};  //* event display manager

  ClassDef(EventDisplay, 1);
};
}  // namespace ldmx

#endif
