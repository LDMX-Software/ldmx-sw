/**
 * @file EventDisplay.cxx
 * @author Josh Hiltbrand, University of Minnesota
 */

#include "EventDisplay/EventDisplay.h"

ClassImp(ldmx::EventDisplay);

namespace ldmx {

    EventDisplay::EventDisplay( TEveManager* manager , bool verbose ) 
        : TGMainFrame(gClient->GetRoot(), 320, 320), verbose_(verbose) {

        SetWindowName("LDMX Event Display");

        manager_ = manager;
        TGLViewer* viewer = manager_->GetDefaultGLViewer();
        viewer->UseLightColorSet();

        theDetector_ = new EveDetectorGeometry();
        eventObjects_ = new EventObjects();

        manager_->AddEvent(new TEveEventManager("LDMX Detector", ""));
        manager_->AddElement(theDetector_->getECAL());
        manager_->AddElement(theDetector_->getHCAL());
        manager_->AddElement(theDetector_->getRecoilTracker());
        manager_->AddEvent(new TEveEventManager("LDMX Event", ""));

        TGVerticalFrame* contents = new TGVerticalFrame(this, 800,1200);
        TGHorizontalFrame* commandFrame1 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame2 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame3 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame5 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame6 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame7 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame8 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame9 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame10 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame11 = new TGHorizontalFrame(contents, 100,0);

        TGButton* buttonColor = new TGTextButton(commandFrame3, "Color Clusters");
        commandFrame3->AddFrame(buttonColor, new TGLayoutHints(kLHintsExpandX));
        buttonColor->Connect("Pressed()", "ldmx::EventDisplay", this, "ColorClusters()");

        TGButton* buttonPrevious = new TGTextButton(commandFrame2, "<<< Previous Event");
        commandFrame2->AddFrame(buttonPrevious, new TGLayoutHints(kLHintsExpandX));
        buttonPrevious->Connect("Pressed()", "ldmx::EventDisplay", this, "PreviousEvent()");

        TGButton* buttonNext = new TGTextButton(commandFrame2, "Next Event >>>");
        commandFrame2->AddFrame(buttonNext, new TGLayoutHints(kLHintsExpandX));
        buttonNext->Connect("Pressed()", "ldmx::EventDisplay", this, "NextEvent()");

        textBoxGotoEvent_ = new TGTextEntry(commandFrame1, new TGTextBuffer(100));
        commandFrame1->AddFrame(textBoxGotoEvent_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonGoTo = new TGTextButton(commandFrame1, "Go to Event");
        commandFrame1->AddFrame(buttonGoTo, new TGLayoutHints(kLHintsExpandX));
        buttonGoTo->Connect("Pressed()", "ldmx::EventDisplay", this, "GotoEvent()");

        textBoxClustersCollName_ = new TGTextEntry(commandFrame5, new TGTextBuffer(100));
        commandFrame5->AddFrame(textBoxClustersCollName_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonClusterName = new TGTextButton(commandFrame5, "Set Clusters Branch");
        commandFrame5->AddFrame(buttonClusterName, new TGLayoutHints(kLHintsExpandX));
        buttonClusterName->Connect("Pressed()", "ldmx::EventDisplay", this, "GetClustersCollInput()");

        textBoxSimThresh_ = new TGTextEntry(commandFrame6, new TGTextBuffer(100));
        commandFrame6->AddFrame(textBoxSimThresh_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonDrawThresh = new TGTextButton(commandFrame6, "Sim P [MeV] Threshold");
        commandFrame6->AddFrame(buttonDrawThresh, new TGLayoutHints(kLHintsExpandX));
        buttonDrawThresh->Connect("Pressed()", "ldmx::EventDisplay", this, "SetSimThresh()");

        textBoxEventTreeName_ = new TGTextEntry(commandFrame7, new TGTextBuffer(100));
        commandFrame7->AddFrame(textBoxEventTreeName_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonSetTree = new TGTextButton(commandFrame7, "Set Event TTree");
        commandFrame7->AddFrame(buttonSetTree, new TGLayoutHints(kLHintsExpandX));
        buttonSetTree->Connect("Pressed()", "ldmx::EventDisplay", this, "SetEventTree()");

        textBoxEcalDigisCollName_ = new TGTextEntry(commandFrame8, new TGTextBuffer(100));
        commandFrame8->AddFrame(textBoxEcalDigisCollName_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonSetECALBranch = new TGTextButton(commandFrame8, "Set ECAL Digis Branch");
        commandFrame8->AddFrame(buttonSetECALBranch, new TGLayoutHints(kLHintsExpandX));
        buttonSetECALBranch->Connect("Pressed()", "ldmx::EventDisplay", this, "GetECALDigisCollInput()");

        textBoxHcalDigisCollName_ = new TGTextEntry(commandFrame9, new TGTextBuffer(100));
        commandFrame9->AddFrame(textBoxHcalDigisCollName_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonSetHCALBranch = new TGTextButton(commandFrame9, "Set HCAL Digis Branch");
        commandFrame9->AddFrame(buttonSetHCALBranch, new TGLayoutHints(kLHintsExpandX));
        buttonSetHCALBranch->Connect("Pressed()", "ldmx::EventDisplay", this, "GetHCALDigisCollInput()");

        textBoxTrackerHitsCollName_ = new TGTextEntry(commandFrame10, new TGTextBuffer(100));
        commandFrame10->AddFrame(textBoxTrackerHitsCollName_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonSetRecoilBranch = new TGTextButton(commandFrame10, "Set Recoil Sims Branch");
        commandFrame10->AddFrame(buttonSetRecoilBranch, new TGLayoutHints(kLHintsExpandX));
        buttonSetRecoilBranch->Connect("Pressed()", "ldmx::EventDisplay", this, "GetTrackerHitsCollInput()");

        textBoxEcalScorePlaneBranch_ = new TGTextEntry(commandFrame11, new TGTextBuffer(100));
        commandFrame11->AddFrame(textBoxEcalScorePlaneBranch_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonSetSimParticlesBranch = new TGTextButton(commandFrame11, "Set Sim Particles Branch");
        commandFrame11->AddFrame(buttonSetSimParticlesBranch, new TGLayoutHints(kLHintsExpandX));
        buttonSetSimParticlesBranch->Connect("Pressed()", "ldmx::EventDisplay", this, "GetEcalSimParticlesCollInput()");

        contents->AddFrame(commandFrame7, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame8, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame9, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame10, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame11, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame5, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame1, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame2, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame3, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame6, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        AddFrame(contents, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        MapSubwindows();
        Resize(GetDefaultSize());
        MapWindow();

        manager_->FullRedraw3D(kTRUE);

    }

    bool EventDisplay::SetFile(const TString file) {

        file_ = TFile::Open(file);

        if (!file_) {
            std::cout << "\n[ EventDisplay ] : Input root file cannot be opened." << std::endl;
            return false;
        }

        tree_ = (TTree*) file_->Get(eventTreeName_);

        if (!tree_) {
            std::cout << "\n[ EventDisplay ] : Input file contains no tree \"" << eventTreeName_ << "\"" << std::endl;
            return false;
        }
        eventNumMax_ = tree_->GetEntriesFast()-1;

        ecalDigiHits_     = new TClonesArray( EventConstants::ECAL_HIT.c_str() );
        hcalDigiHits_     = new TClonesArray( EventConstants::HCAL_HIT.c_str() );
        recoilHits_       = new TClonesArray( EventConstants::SIM_TRACKER_HIT.c_str() );
        ecalClusters_     = new TClonesArray( EventConstants::ECAL_CLUSTER.c_str() );
        ecalSimParticles_ = new TClonesArray( EventConstants::SIM_TRACKER_HIT.c_str() );

        foundECALDigis_     = GetCollection( ecalDigisCollName_ , ecalDigiHits_ );
        foundHCALDigis_     = GetCollection( hcalDigisCollName_ , hcalDigiHits_ );
        foundClusters_      = GetCollection( clustersCollName_ , ecalClusters_ );
        foundTrackerHits_   = GetCollection( trackerHitsCollName_ , recoilHits_ );
        foundEcalSPHits_    = GetCollection( ecalSimParticlesCollName_ , ecalSimParticles_ );

        return true;
    }

    void EventDisplay::PreviousEvent() {
        if (eventNum_ <= 0) {
            return;
        }

        GotoEvent(eventNum_ - 1);
    }

    void EventDisplay::NextEvent() {
        if (eventNumMax_ == eventNum_) {
            return;
        }

        GotoEvent(eventNum_ + 1);
    }

    bool EventDisplay::GetCollection( const TString branchName , TClonesArray *collection ) {
        
        if ( tree_->GetListOfBranches()->FindObject(branchName) ) {
            tree_->SetBranchAddress( branchName , &collection );
            return true;
        } else {
            std::cout << "[ EventDisplay ] : No branch with name \"" << branchName << "\"" << std::endl;
            return false;
        }
    }

    void EventDisplay::GetECALDigisCollInput() {

        const TString ecalDigisCollName = textBoxEcalDigisCollName_->GetText();
        foundECALDigis_ = GetCollection( ecalDigisCollName , ecalDigiHits_ );

        if (foundECALDigis_) {
            ecalDigisCollName_ = ecalDigisCollName;
            if (eventNum_ != -1) {
                GotoEvent(eventNum_);
            }
        }
    }

    void EventDisplay::GetHCALDigisCollInput() {

        const TString hcalDigisCollName = textBoxHcalDigisCollName_->GetText();
        foundHCALDigis_ = GetCollection( hcalDigisCollName , hcalDigiHits_ );

        if (foundHCALDigis_) {
            hcalDigisCollName_ = hcalDigisCollName;
            if (eventNum_ != -1) {
                GotoEvent(eventNum_);
            }
        }
    }

    void EventDisplay::GetTrackerHitsCollInput() {

        const TString trackerHitsCollName = textBoxTrackerHitsCollName_->GetText();
        foundTrackerHits_ = GetCollection( trackerHitsCollName , recoilHits_ );

        if (foundTrackerHits_) {
            trackerHitsCollName_ = trackerHitsCollName;
            if (eventNum_ != -1) {
                GotoEvent(eventNum_);
            }
        }
    }

    void EventDisplay::GetClustersCollInput() {

        const TString clustersCollName = textBoxClustersCollName_->GetText();
        foundClusters_ = GetCollection( clustersCollName , ecalClusters_ );

        if (foundClusters_) {
            clustersCollName_ = clustersCollName;
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

        if ( verbose_ ) {
            std::cout << "[ EventDisplay ] : Loading event " << event << "... " << std::flush;
        }

        tree_->GetEntry(eventNum_);

        if (foundECALDigis_) {
            eventObjects_->drawECALHits(ecalDigiHits_);
        }

        if (foundHCALDigis_) {
            eventObjects_->drawHCALHits(hcalDigiHits_);
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

        if ( verbose_ ) {
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
        TTree* tree = (TTree*) file_->Get(treeName);
        if (!tree) {
            std::cout << "\n[ EventDisplay ] : Input file contains no tree \"" << treeName << "\"" << std::endl;
            return false;
        }

        tree_ = tree;
        eventTreeName_ = treeName;

        foundECALDigis_     = GetCollection( ecalDigisCollName_ , ecalDigiHits_ );
        foundHCALDigis_     = GetCollection( hcalDigisCollName_ , hcalDigiHits_ );
        foundClusters_      = GetCollection( clustersCollName_ , ecalClusters_ );
        foundTrackerHits_   = GetCollection( trackerHitsCollName_ , recoilHits_ );
        foundEcalSPHits_    = GetCollection( ecalSimParticlesCollName_ , ecalSimParticles_ );

        return GotoEvent(eventNum_);
    }

    bool EventDisplay::SetSimThresh() {

        double thresh = atof(textBoxSimThresh_->GetText());
        if (thresh == 0 && std::string(textBoxSimThresh_->GetText()) != "0") {
            std::cout << "[ EventDisplay ] : Invalid sim energy threshold entered: \""
                << textBoxSimThresh_->GetText() << "\"" << std::endl;
            return false;
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
}
