/**
 * @file Display.cxx
 * @author Josh Hiltbrand, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "EventDisplay/Display.h"

ClassImp(eventdisplay::Display);

namespace eventdisplay {

Display::Display(TEveManager* manager, bool verbose)
    : TGMainFrame(gClient->GetRoot(), 320, 320), verbose_(verbose) {
  /****************************************************************************
   * GUI Set Up
   ***************************************************************************/

  SetWindowName("LDMX Event Display");

  manager_ = manager;
  TGLViewer* viewer = manager_->GetDefaultGLViewer();
  viewer->UseLightColorSet();

  if (verbose_) {
    std::cout << "[ Display ] : Drawing detector geometry... " << std::flush;
  }

  // when the first TGeoShape (a TGeoTube) is drawn, ROOT creates a default
  // geometry for this drawing and
  // prints an Info statement to std-out. Currently, I can't figure out how to
  // turn this behavior off.
  the_detector_ = new EveDetectorGeometry();

  manager_->AddEvent(new TEveEventManager("LDMX Detector", ""));
  manager_->AddElement(the_detector_->getECAL());
  manager_->AddElement(the_detector_->getHCAL());
  manager_->AddElement(the_detector_->getRecoilTracker());
  manager_->AddEvent(new TEveEventManager("LDMX Event", ""));

  if (verbose_) {
    std::cout << "done" << std::endl;
    std::cout << "[ Display ] : Constructing and linking buttons... "
              << std::flush;
  }

  TGVerticalFrame* contents = new TGVerticalFrame(this, 1000, 1200);
  TGHorizontalFrame* commandFrameNextEvent =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameColorClusters =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameEcalClusterBranch =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameSimThresh =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameEcalHitBranch =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameHcalHitBranch =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameTrackerHitsBranch =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameEcalScorePlaneBranch =
      new TGHorizontalFrame(contents, 100, 0);

  TGButton* buttonColor =
      new TGTextButton(commandFrameColorClusters, "Color Clusters");
  commandFrameColorClusters->AddFrame(buttonColor,
                                      new TGLayoutHints(kLHintsExpandX));
  buttonColor->Connect("Pressed()", "eventdisplay::Display", this,
                       "ColorClusters()");

  TGButton* buttonNext =
      new TGTextButton(commandFrameNextEvent, "Next Event >>>");
  commandFrameNextEvent->AddFrame(buttonNext,
                                  new TGLayoutHints(kLHintsExpandX));
  buttonNext->Connect("Pressed()", "eventdisplay::Display", this,
                      "NextEvent()");

  text_box_clusters_coll_name_ =
      new TGTextEntry(commandFrameEcalClusterBranch, new TGTextBuffer(100));
  commandFrameEcalClusterBranch->AddFrame(text_box_clusters_coll_name_,
                                          new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonClusterName =
      new TGTextButton(commandFrameEcalClusterBranch, "Set Clusters Branch");
  commandFrameEcalClusterBranch->AddFrame(buttonClusterName,
                                          new TGLayoutHints(kLHintsExpandX));
  buttonClusterName->Connect("Pressed()", "eventdisplay::Display", this,
                             "GetClustersCollInput()");

  text_box_sim_thresh_ =
      new TGTextEntry(commandFrameSimThresh, new TGTextBuffer(100));
  commandFrameSimThresh->AddFrame(text_box_sim_thresh_,
                                  new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonDrawThresh =
      new TGTextButton(commandFrameSimThresh, "Sim P [MeV] Threshold");
  commandFrameSimThresh->AddFrame(buttonDrawThresh,
                                  new TGLayoutHints(kLHintsExpandX));
  buttonDrawThresh->Connect("Pressed()", "eventdisplay::Display", this,
                            "SetSimThresh()");

  text_box_ecal_rec_hits_coll_name_ =
      new TGTextEntry(commandFrameEcalHitBranch, new TGTextBuffer(100));
  commandFrameEcalHitBranch->AddFrame(text_box_ecal_rec_hits_coll_name_,
                                      new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetECALBranch =
      new TGTextButton(commandFrameEcalHitBranch, "Set ECAL RecHits Branch");
  commandFrameEcalHitBranch->AddFrame(buttonSetECALBranch,
                                      new TGLayoutHints(kLHintsExpandX));
  buttonSetECALBranch->Connect("Pressed()", "eventdisplay::Display", this,
                               "GetECALRecHitsCollInput()");

  text_box_hcal_rec_hits_coll_name_ =
      new TGTextEntry(commandFrameHcalHitBranch, new TGTextBuffer(100));
  commandFrameHcalHitBranch->AddFrame(text_box_hcal_rec_hits_coll_name_,
                                      new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetHCALBranch =
      new TGTextButton(commandFrameHcalHitBranch, "Set HCAL RecHits Branch");
  commandFrameHcalHitBranch->AddFrame(buttonSetHCALBranch,
                                      new TGLayoutHints(kLHintsExpandX));
  buttonSetHCALBranch->Connect("Pressed()", "eventdisplay::Display", this,
                               "GetHCALRecHitsCollInput()");

  text_box_tracker_hits_coll_name_ =
      new TGTextEntry(commandFrameTrackerHitsBranch, new TGTextBuffer(100));
  commandFrameTrackerHitsBranch->AddFrame(text_box_tracker_hits_coll_name_,
                                          new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetRecoilBranch =
      new TGTextButton(commandFrameTrackerHitsBranch, "Set Recoil Sims Branch");
  commandFrameTrackerHitsBranch->AddFrame(buttonSetRecoilBranch,
                                          new TGLayoutHints(kLHintsExpandX));
  buttonSetRecoilBranch->Connect("Pressed()", "eventdisplay::Display", this,
                                 "GetTrackerHitsCollInput()");

  text_box_ecal_score_plane_branch_ =
      new TGTextEntry(commandFrameEcalScorePlaneBranch, new TGTextBuffer(100));
  commandFrameEcalScorePlaneBranch->AddFrame(text_box_ecal_score_plane_branch_,
                                             new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetSimParticlesBranch =
      new TGTextButton(commandFrameEcalScorePlaneBranch, "Set Ecal SP Branch");
  commandFrameEcalScorePlaneBranch->AddFrame(buttonSetSimParticlesBranch,
                                             new TGLayoutHints(kLHintsExpandX));
  buttonSetSimParticlesBranch->Connect("Pressed()", "eventdisplay::Display",
                                       this, "GetEcalSimParticlesCollInput()");

  // Order from top to bottom here
  contents->AddFrame(commandFrameEcalHitBranch,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameHcalHitBranch,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameTrackerHitsBranch,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameEcalScorePlaneBranch,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameEcalClusterBranch,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameNextEvent,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameColorClusters,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameSimThresh,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  AddFrame(contents, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  MapSubwindows();
  Resize(GetDefaultSize());
  MapWindow();

  manager_->FullRedraw3D(kTRUE);

  if (verbose_) {
    std::cout << "done" << std::endl;
  }
}

bool Display::SetFile(const TString file) {
  try {
    framework::config::Parameters file_config;
    file_config.addParameter("tree_name", std::string("LDMX_Events"));
    the_file_ =
        std::make_unique<framework::EventFile>(file_config, file.Data());
    the_file_->setupEvent(&the_event_);
    if (verbose_) {
      std::cout << "[ Display ] : Input root file opened successfully."
                << std::endl;
    }
  } catch (const framework::exception::Exception& e) {
    std::cerr << "[ Display ] : Input root file cannot be opened." << std::endl;
    std::cerr << "[" << e.name() << "] : " << e.message() << "\n"
              << "  at " << e.module() << ":" << e.line() << " in "
              << e.function() << "\nStack trace: " << std::endl
              << e.stackTrace();
    return false;
  }

  return true;
}

void Display::NextEvent() {
  if (the_file_->nextEvent(false)) {
    manager_->GetCurrentEvent()->DestroyElements();
    objects_.Initialize();

    if (verbose_) {
      std::cout << "[ Display ] : Loading new event " << "... " << std::endl;
    }

    // draw<std::vector<ldmx::EcalHit>>(ecal_rec_hits_coll_name_);
    // draw<std::vector<ldmx::HcalHit>>(hcal_rec_hits_coll_name_);
    // draw<std::vector<ldmx::EcalCluster>>(clusters_coll_name_);
    draw<std::vector<ldmx::SimTrackerHit>>(tracker_hits_coll_name_);
    draw<std::vector<ldmx::SimTrackerHit>>(ecal_sim_particles_coll_name_);
    draw<std::vector<ldmx::SimCalorimeterHit>>("EcalSimHits");
    draw<std::vector<ldmx::SimCalorimeterHit>>("HcalSimHits");
    draw<std::map<int, ldmx::SimParticle>>("SimParticles");

    if (verbose_)
      std::cout << "[ Display ] : Done loading event objects into EVE objects."
                << std::endl;

    manager_->AddElement(objects_.getSimObjects());
    manager_->AddElement(objects_.getRecObjects());
    manager_->Redraw3D(kFALSE);

    if (verbose_) std::cout << "[ Display ] : Done redrawing." << std::endl;

  } else {
    std::cout << "[ Display ] : Already at last event in file." << std::endl;
    return;
  }
}

void Display::GetECALRecHitsCollInput() {
  ecal_rec_hits_coll_name_ = getText(text_box_ecal_rec_hits_coll_name_);
}

void Display::GetHCALRecHitsCollInput() {
  hcal_rec_hits_coll_name_ = getText(text_box_hcal_rec_hits_coll_name_);
}

void Display::GetTrackerHitsCollInput() {
  tracker_hits_coll_name_ = getText(text_box_tracker_hits_coll_name_);
}

void Display::GetClustersCollInput() {
  clusters_coll_name_ = getText(text_box_clusters_coll_name_);
}

void Display::GetEcalSimParticlesCollInput() {
  ecal_sim_particles_coll_name_ = getText(text_box_ecal_score_plane_branch_);
}

bool Display::SetSimThresh() {
  double thresh = atof(text_box_sim_thresh_->GetText());
  if (thresh == 0 && std::string(text_box_sim_thresh_->GetText()) != "0") {
    std::cout << "[ Display ] : Invalid sim energy threshold entered: \""
              << text_box_sim_thresh_->GetText() << "\"" << std::endl;
    return false;
  } else if (verbose_) {
    std::cout << "[ Display ] : Setting SimParticle energy threshold to "
              << thresh << std::endl;
  }

  objects_.SetSimThresh(thresh);

  manager_->RegisterRedraw3D();
  manager_->FullRedraw3D(kFALSE, kTRUE);

  return true;
}

void Display::ColorClusters() {
  objects_.ColorClusters();

  manager_->RegisterRedraw3D();
  manager_->FullRedraw3D(kFALSE, kTRUE);
}

}  // namespace eventdisplay
