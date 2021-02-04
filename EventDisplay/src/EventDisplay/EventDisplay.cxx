/**
 * @file EventDisplay.cxx
 * @author Josh Hiltbrand, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "EventDisplay/EventDisplay.h"

ClassImp(ldmx::EventDisplay);

namespace ldmx {

EventDisplay::EventDisplay(TEveManager* manager, bool verbose)
    : TGMainFrame(gClient->GetRoot(), 320, 320), verbose_(verbose) {
  SetWindowName("LDMX Event Display");

  manager_ = manager;
  TGLViewer* viewer = manager_->GetDefaultGLViewer();
  viewer->UseLightColorSet();

  if (verbose_) {
    std::cout << "[ EventDisplay ] : Drawing detector geometry... "
              << std::flush;
  }

  // when the first TGeoShape (a TGeoTube) is drawn, ROOT creates a default
  // geometry for this drawing and
  // prints an Info statement to std-out. Currently, I can't figure out how to
  // turn this behavior off.
  theDetector_ = new EveDetectorGeometry();
  eventObjects_ = new EventObjects();

  manager_->AddEvent(new TEveEventManager("LDMX Detector", ""));
  manager_->AddElement(theDetector_->getECAL());
  manager_->AddElement(theDetector_->getHCAL());
  manager_->AddElement(theDetector_->getRecoilTracker());
  manager_->AddEvent(new TEveEventManager("LDMX Event", ""));

  if (verbose_) {
    std::cout << "done" << std::endl;
    std::cout << "[ EventDisplay ] : Constructing and linking buttons... "
              << std::flush;
  }

  TGVerticalFrame* contents = new TGVerticalFrame(this, 1000, 1200);
  TGHorizontalFrame* commandFrameGotoEvent =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameNextEvent =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameColorClusters =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameEcalClusterBranch =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameSimThresh =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameEventTree =
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
  buttonColor->Connect("Pressed()", "ldmx::EventDisplay", this,
                       "ColorClusters()");

  TGButton* buttonPrevious =
      new TGTextButton(commandFrameNextEvent, "<<< Previous Event");
  commandFrameNextEvent->AddFrame(buttonPrevious,
                                  new TGLayoutHints(kLHintsExpandX));
  buttonPrevious->Connect("Pressed()", "ldmx::EventDisplay", this,
                          "PreviousEvent()");

  TGButton* buttonNext =
      new TGTextButton(commandFrameNextEvent, "Next Event >>>");
  commandFrameNextEvent->AddFrame(buttonNext,
                                  new TGLayoutHints(kLHintsExpandX));
  buttonNext->Connect("Pressed()", "ldmx::EventDisplay", this, "NextEvent()");

  textBoxGotoEvent_ =
      new TGTextEntry(commandFrameGotoEvent, new TGTextBuffer(100));
  commandFrameGotoEvent->AddFrame(textBoxGotoEvent_,
                                  new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonGoTo = new TGTextButton(commandFrameGotoEvent, "Go to Event");
  commandFrameGotoEvent->AddFrame(buttonGoTo,
                                  new TGLayoutHints(kLHintsExpandX));
  buttonGoTo->Connect("Pressed()", "ldmx::EventDisplay", this, "GotoEvent()");

  textBoxClustersCollName_ =
      new TGTextEntry(commandFrameEcalClusterBranch, new TGTextBuffer(100));
  commandFrameEcalClusterBranch->AddFrame(textBoxClustersCollName_,
                                          new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonClusterName =
      new TGTextButton(commandFrameEcalClusterBranch, "Set Clusters Branch");
  commandFrameEcalClusterBranch->AddFrame(buttonClusterName,
                                          new TGLayoutHints(kLHintsExpandX));
  buttonClusterName->Connect("Pressed()", "ldmx::EventDisplay", this,
                             "GetClustersCollInput()");

  textBoxSimThresh_ =
      new TGTextEntry(commandFrameSimThresh, new TGTextBuffer(100));
  commandFrameSimThresh->AddFrame(textBoxSimThresh_,
                                  new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonDrawThresh =
      new TGTextButton(commandFrameSimThresh, "Sim P [MeV] Threshold");
  commandFrameSimThresh->AddFrame(buttonDrawThresh,
                                  new TGLayoutHints(kLHintsExpandX));
  buttonDrawThresh->Connect("Pressed()", "ldmx::EventDisplay", this,
                            "SetSimThresh()");

  textBoxEventTreeName_ =
      new TGTextEntry(commandFrameEventTree, new TGTextBuffer(100));
  commandFrameEventTree->AddFrame(textBoxEventTreeName_,
                                  new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetTree =
      new TGTextButton(commandFrameEventTree, "Set Event TTree");
  commandFrameEventTree->AddFrame(buttonSetTree,
                                  new TGLayoutHints(kLHintsExpandX));
  buttonSetTree->Connect("Pressed()", "ldmx::EventDisplay", this,
                         "SetEventTree()");

  textBoxEcalRecHitsCollName_ =
      new TGTextEntry(commandFrameEcalHitBranch, new TGTextBuffer(100));
  commandFrameEcalHitBranch->AddFrame(textBoxEcalRecHitsCollName_,
                                      new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetECALBranch =
      new TGTextButton(commandFrameEcalHitBranch, "Set ECAL RecHits Branch");
  commandFrameEcalHitBranch->AddFrame(buttonSetECALBranch,
                                      new TGLayoutHints(kLHintsExpandX));
  buttonSetECALBranch->Connect("Pressed()", "ldmx::EventDisplay", this,
                               "GetECALRecHitsCollInput()");

  textBoxHcalRecHitsCollName_ =
      new TGTextEntry(commandFrameHcalHitBranch, new TGTextBuffer(100));
  commandFrameHcalHitBranch->AddFrame(textBoxHcalRecHitsCollName_,
                                      new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetHCALBranch =
      new TGTextButton(commandFrameHcalHitBranch, "Set HCAL RecHits Branch");
  commandFrameHcalHitBranch->AddFrame(buttonSetHCALBranch,
                                      new TGLayoutHints(kLHintsExpandX));
  buttonSetHCALBranch->Connect("Pressed()", "ldmx::EventDisplay", this,
                               "GetHCALRecHitsCollInput()");

  textBoxTrackerHitsCollName_ =
      new TGTextEntry(commandFrameTrackerHitsBranch, new TGTextBuffer(100));
  commandFrameTrackerHitsBranch->AddFrame(textBoxTrackerHitsCollName_,
                                          new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetRecoilBranch =
      new TGTextButton(commandFrameTrackerHitsBranch, "Set Recoil Sims Branch");
  commandFrameTrackerHitsBranch->AddFrame(buttonSetRecoilBranch,
                                          new TGLayoutHints(kLHintsExpandX));
  buttonSetRecoilBranch->Connect("Pressed()", "ldmx::EventDisplay", this,
                                 "GetTrackerHitsCollInput()");

  textBoxEcalScorePlaneBranch_ =
      new TGTextEntry(commandFrameEcalScorePlaneBranch, new TGTextBuffer(100));
  commandFrameEcalScorePlaneBranch->AddFrame(textBoxEcalScorePlaneBranch_,
                                             new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetSimParticlesBranch = new TGTextButton(
      commandFrameEcalScorePlaneBranch, "Set Sim Particles Branch");
  commandFrameEcalScorePlaneBranch->AddFrame(buttonSetSimParticlesBranch,
                                             new TGLayoutHints(kLHintsExpandX));
  buttonSetSimParticlesBranch->Connect("Pressed()", "ldmx::EventDisplay", this,
                                       "GetEcalSimParticlesCollInput()");

  // Order from top to bottom here
  contents->AddFrame(commandFrameEventTree,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
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
  contents->AddFrame(commandFrameGotoEvent,
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

bool EventDisplay::SetFile(const TString file) {
  file_ = TFile::Open(file);

  if (!file_) {
    std::cout << "[ EventDisplay ] : Input root file cannot be opened."
              << std::endl;
    return false;
  } else if (verbose_) {
    std::cout << "[ EventDisplay ] : Input root file opened successfully."
              << std::endl;
  }

  tree_ = (TTree*)file_->Get(eventTreeName_);

  if (!tree_) {
    std::cout << "[ EventDisplay ] : Input file contains no tree \""
              << eventTreeName_ << "\"" << std::endl;
    return false;
  } else if (verbose_) {
    std::cout << "[ EventDisplay ] : Event tree retrieved from input file."
              << std::endl;
  }

  eventNumMax_ = tree_->GetEntriesFast() - 1;

  foundECALRecHits_ = GetCollection(ecalRecHitsCollName_, ecalRecHits_);
  foundHCALRecHits_ = GetCollection(hcalRecHitsCollName_, hcalRecHits_);
  foundClusters_ = GetCollection(clustersCollName_, ecalClusters_);
  foundTrackerHits_ = GetCollection(trackerHitsCollName_, recoilHits_);
  foundEcalSPHits_ =
      GetCollection(ecalSimParticlesCollName_, ecalSimParticles_);

  return true;
}

void EventDisplay::PreviousEvent() {
  if (eventNum_ <= 0) {
    std::cout << "[ EventDisplay ] : Already at first event in file."
              << std::endl;
    return;
  }

  GotoEvent(eventNum_ - 1);
}

void EventDisplay::NextEvent() {
  if (eventNumMax_ == eventNum_) {
    std::cout << "[ EventDisplay ] : Already at last event in file."
              << std::endl;
    return;
  }

  GotoEvent(eventNum_ + 1);
}

void EventDisplay::GetECALRecHitsCollInput() {
  const TString ecalRecHitsCollName = textBoxEcalRecHitsCollName_->GetText();
  foundECALRecHits_ = GetCollection(ecalRecHitsCollName, ecalRecHits_);

  if (foundECALRecHits_) {
    ecalRecHitsCollName_ = ecalRecHitsCollName;
    if (eventNum_ != -1) {
      GotoEvent(eventNum_);
    }
  }
}

void EventDisplay::GetHCALRecHitsCollInput() {
  const TString hcalRecHitsCollName = textBoxHcalRecHitsCollName_->GetText();
  foundHCALRecHits_ = GetCollection(hcalRecHitsCollName, hcalRecHits_);

  if (foundHCALRecHits_) {
    hcalRecHitsCollName_ = hcalRecHitsCollName;
    if (eventNum_ != -1) {
      GotoEvent(eventNum_);
    }
  }
}

void EventDisplay::GetTrackerHitsCollInput() {
  const TString trackerHitsCollName = textBoxTrackerHitsCollName_->GetText();
  foundTrackerHits_ = GetCollection(trackerHitsCollName, recoilHits_);

  if (foundTrackerHits_) {
    trackerHitsCollName_ = trackerHitsCollName;
    if (eventNum_ != -1) {
      GotoEvent(eventNum_);
    }
  }
}

void EventDisplay::GetClustersCollInput() {
  const TString clustersCollName = textBoxClustersCollName_->GetText();
  foundClusters_ = GetCollection(clustersCollName, ecalClusters_);

  if (foundClusters_) {
    clustersCollName_ = clustersCollName;
    if (eventNum_ != -1) {
      GotoEvent(eventNum_);
    }
  }
}

void EventDisplay::GetEcalSimParticlesCollInput() {
  const TString ecalSimParticlesCollName =
      textBoxEcalScorePlaneBranch_->GetText();
  foundEcalSPHits_ = GetCollection(ecalSimParticlesCollName, ecalSimParticles_);

  if (foundEcalSPHits_) {
    ecalSimParticlesCollName_ = ecalSimParticlesCollName;
    if (eventNum_ != -1) {
      GotoEvent(eventNum_);
    }
  }
}

bool EventDisplay::GotoEvent(int event) {
  manager_->GetCurrentEvent()->DestroyElements();
  eventObjects_->Initialize();

  if (event > eventNumMax_ || event < 0) {
    std::cout << "[ EventDisplay ] : Event number out of range." << std::endl;
    return false;
  }
  eventNum_ = event;

  if (verbose_) {
    std::cout << "[ EventDisplay ] : Loading event " << event << "... "
              << std::flush;
  }

  tree_->GetEntry(eventNum_);

  if (foundECALRecHits_) {
    eventObjects_->drawECALHits(ecalRecHits_);
  }

  if (foundHCALRecHits_) {
    eventObjects_->drawHCALHits(hcalRecHits_);
  }

  if (foundClusters_) {
    eventObjects_->drawECALClusters(ecalClusters_);
  }

  if (foundTrackerHits_) {
    eventObjects_->drawRecoilHits(recoilHits_);
  }

  if (foundEcalSPHits_) {
    eventObjects_->drawECALSimParticles(ecalSimParticles_);
  }

  manager_->AddElement(eventObjects_->getHitCollections());
  manager_->AddElement(eventObjects_->getRecoObjects());
  manager_->Redraw3D(kFALSE);

  if (verbose_) {
    std::cout << "done" << std::endl;
  }

  return true;
}

bool EventDisplay::GotoEvent() {
  int event = atoi(textBoxGotoEvent_->GetText());
  if (event == 0 && std::string(textBoxGotoEvent_->GetText()) != "0") {
    std::cout << "[ EventDisplay ] : Invalid event number entered: \""
              << textBoxGotoEvent_->GetText() << "\"" << std::endl;
    return false;
  }

  return GotoEvent(event);
}

bool EventDisplay::SetEventTree() {
  const TString treeName = textBoxEventTreeName_->GetText();
  TTree* tree = (TTree*)file_->Get(treeName);
  if (!tree) {
    std::cout << "[ EventDisplay ] : Input file contains no tree \"" << treeName
              << "\"" << std::endl;
    return false;
  } else if (verbose_) {
    std::cout << "[ EventDisplay ] : Event tree set to \"" << treeName << "\""
              << std::endl;
  }

  tree_ = tree;
  eventTreeName_ = treeName;

  foundECALRecHits_ = GetCollection(ecalRecHitsCollName_, ecalRecHits_);
  foundHCALRecHits_ = GetCollection(hcalRecHitsCollName_, hcalRecHits_);
  foundClusters_ = GetCollection(clustersCollName_, ecalClusters_);
  foundTrackerHits_ = GetCollection(trackerHitsCollName_, recoilHits_);
  foundEcalSPHits_ =
      GetCollection(ecalSimParticlesCollName_, ecalSimParticles_);

  return GotoEvent(eventNum_);
}

bool EventDisplay::SetSimThresh() {
  double thresh = atof(textBoxSimThresh_->GetText());
  if (thresh == 0 && std::string(textBoxSimThresh_->GetText()) != "0") {
    std::cout << "[ EventDisplay ] : Invalid sim energy threshold entered: \""
              << textBoxSimThresh_->GetText() << "\"" << std::endl;
    return false;
  } else if (verbose_) {
    std::cout << "[ EventDisplay ] : Setting SimParticle energy threshold to "
              << thresh << std::endl;
  }

  eventObjects_->SetSimThresh(thresh);

  manager_->RegisterRedraw3D();
  manager_->FullRedraw3D(kFALSE, kTRUE);

  return true;
}

void EventDisplay::ColorClusters() {
  eventObjects_->ColorClusters();

  manager_->RegisterRedraw3D();
  manager_->FullRedraw3D(kFALSE, kTRUE);
}

}  // namespace ldmx
