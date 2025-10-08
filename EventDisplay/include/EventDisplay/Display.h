/**
 * @file Display.h
 * @author Josh Hiltbrand, University of Minnesota
 */

#ifndef EVENTDISPLAY_EVENTDISPLAY_H_
#define EVENTDISPLAY_EVENTDISPLAY_H_

#include <iostream>

#include "EventDisplay/EveDetectorGeometry.h"
#include "EventDisplay/Objects.h"
#include "Framework/Event.h"
#include "Framework/EventFile.h"
#include "TBranchElement.h"
#include "TEveBrowser.h"
#include "TEveElement.h"
#include "TEveEventManager.h"
#include "TEveManager.h"
#include "TEveViewer.h"
#include "TFile.h"
#include "TGButton.h"
#include "TGFrame.h"
#include "TGLViewer.h"
#include "TGTextEntry.h"
#include "TRint.h"
#include "TString.h"
#include "TTree.h"

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
    // need to delete EventFile so that it is deleted before Event
    the_file_.reset(nullptr);

    delete theDetector_;

    delete text_box_clusters_coll_name_;
    delete text_box_sim_thresh_;
    delete text_box_ecal_rec_hits_coll_name_;
    delete text_box_hcal_rec_hits_coll_name_;
    delete text_box_tracker_hits_coll_name_;
    delete text_box_ecal_score_plane_branch_;

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
  bool setFile(const TString file);

  /**
   * Goes forward one event unless the current event number equals the maximum
   * event number.
   */
  void nextEvent();

  /**
   * Gets ecalRecHits collection name from text box
   */
  void getEcalRecHitsCollInput();

  /**
   * Gets clusters collection name from text box
   */
  void getClustersCollInput();

  /**
   * Gets trackerHits collection name from text box
   */
  void getTrackerHitsCollInput();

  /**
   * Gets hcalRecHits collection name from text box
   */
  void getHcalRecHitsCollInput();

  /**
   * Gets ECAL Sim Particles Branch name from text box
   */
  void getEcalSimParticlesCollInput();

  /**
   * Sets threshold energy from a SimParticle to be drawn from text box.
   * Re-draws the display.
   * @return bool success check
   */
  bool setSimThresh();

  /**
   * Colors cluster objects and redraws.
   */
  void colorClusters();

 private:
  /**
   * Get the text from the input text box
   */
  std::string getText(TGTextEntry* box) const { return box->GetText(); }

  /**
   * Templated draw method
   */
  template <typename EventObjectType>
  void draw(const std::string& name) {
    try {
      auto event_object{the_event_.getObject<EventObjectType>(name)};
      objects_.draw(event_object);
      if (verbose_) {
        std::cout << "[ Display ] : Loaded '" << name
                  << "' into memory as a EVE object." << std::endl;
      }
    } catch (const framework::exception::Exception& e) {
      std::cerr << "[ Display ] : Unable to draw an event object." << std::endl;
      std::cerr << "[" << e.name() << "] : " << e.message() << "\n"
                << "  at " << e.module() << ":" << e.line() << " in "
                << e.function() << std::endl;
    }
  }

 private:
  bool verbose_;  //* verbosity flag

  /// Event bus for reading from input file
  framework::Event the_event_{"display"};

  /// Handle to input file we will be reading
  std::unique_ptr<framework::EventFile> the_file_;

  /// name of ecal clusters collection in event tree
  std::string clusters_coll_name_ = "ecalClusters";
  /// name of ecalRecHits collection in event tree
  std::string ecal_rec_hits_coll_name_ = "EcalRecHits";
  /// name of hcalRecHits collection in event tree
  std::string hcal_rec_hits_coll_name_ = "HcalRecHits";
  /// name of recoil hitss collection in event tree
  std::string tracker_hits_coll_name_ = "RecoilSimHits";
  /// name of ecal sim particles collection in
  std::string ecal_sim_particles_coll_name_ = "EcalScoringPlaneHits";

  /// drawing methods for the detector geometry
  EveDetectorGeometry* the_detector_{nullptr};

  /// drawing methods for event objects
  Objects objects_;

  TGTextEntry* text_box_clusters_coll_name_;
  TGTextEntry* text_box_sim_thresh_;
  TGTextEntry* text_box_ecal_rec_hits_coll_name_;
  TGTextEntry* text_box_hcal_rec_hits_coll_name_;
  TGTextEntry* text_box_tracker_hits_coll_name_;
  TGTextEntry* text_box_ecal_score_plane_branch_;

  /// event display manager
  TEveManager* manager_{nullptr};

  ClassDef(Display, 2);
};
}  // namespace eventdisplay

#endif
