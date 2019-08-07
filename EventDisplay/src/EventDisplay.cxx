#include "EventDisplay/EventDisplay.h"

ClassImp(ldmx::EventDisplay);

namespace ldmx {

    EventDisplay::EventDisplay(TEveManager* manager) : TGMainFrame(gClient->GetRoot(), 320, 320) {

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

        textBox1_ = new TGTextEntry(commandFrame1, new TGTextBuffer(100));
        commandFrame1->AddFrame(textBox1_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonGoTo = new TGTextButton(commandFrame1, "Go to Event");
        commandFrame1->AddFrame(buttonGoTo, new TGLayoutHints(kLHintsExpandX));
        buttonGoTo->Connect("Pressed()", "ldmx::EventDisplay", this, "GotoEvent()");

        textBox2_ = new TGTextEntry(commandFrame5, new TGTextBuffer(100));
        commandFrame5->AddFrame(textBox2_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonClusterName = new TGTextButton(commandFrame5, "Set Clusters Branch");
        commandFrame5->AddFrame(buttonClusterName, new TGLayoutHints(kLHintsExpandX));
        buttonClusterName->Connect("Pressed()", "ldmx::EventDisplay", this, "GetClustersCollInput()");

        textBox3_ = new TGTextEntry(commandFrame6, new TGTextBuffer(100));
        commandFrame6->AddFrame(textBox3_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonDrawThresh = new TGTextButton(commandFrame6, "Sim P [MeV] Threshold");
        commandFrame6->AddFrame(buttonDrawThresh, new TGLayoutHints(kLHintsExpandX));
        buttonDrawThresh->Connect("Pressed()", "ldmx::EventDisplay", this, "SetSimThresh()");

        textBox4_ = new TGTextEntry(commandFrame7, new TGTextBuffer(100));
        commandFrame7->AddFrame(textBox4_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonSetTree = new TGTextButton(commandFrame7, "Set Event TTree");
        commandFrame7->AddFrame(buttonSetTree, new TGLayoutHints(kLHintsExpandX));
        buttonSetTree->Connect("Pressed()", "ldmx::EventDisplay", this, "SetEventTree()");

        textBox5_ = new TGTextEntry(commandFrame8, new TGTextBuffer(100));
        commandFrame8->AddFrame(textBox5_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonSetECALBranch = new TGTextButton(commandFrame8, "Set ECAL Digis Branch");
        commandFrame8->AddFrame(buttonSetECALBranch, new TGLayoutHints(kLHintsExpandX));
        buttonSetECALBranch->Connect("Pressed()", "ldmx::EventDisplay", this, "GetECALDigisCollInput()");

        textBox6_ = new TGTextEntry(commandFrame9, new TGTextBuffer(100));
        commandFrame9->AddFrame(textBox6_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonSetHCALBranch = new TGTextButton(commandFrame9, "Set HCAL Digis Branch");
        commandFrame9->AddFrame(buttonSetHCALBranch, new TGLayoutHints(kLHintsExpandX));
        buttonSetHCALBranch->Connect("Pressed()", "ldmx::EventDisplay", this, "GetHCALDigisCollInput()");

        textBox7_ = new TGTextEntry(commandFrame10, new TGTextBuffer(100));
        commandFrame10->AddFrame(textBox7_, new TGLayoutHints(kLHintsExpandX));

        TGButton* buttonSetRecoilBranch = new TGTextButton(commandFrame10, "Set Recoil Sims Branch");
        commandFrame10->AddFrame(buttonSetRecoilBranch, new TGLayoutHints(kLHintsExpandX));
        buttonSetRecoilBranch->Connect("Pressed()", "ldmx::EventDisplay", this, "GetTrackerHitsCollInput()");

        textBox11_ = new TGTextEntry(commandFrame11, new TGTextBuffer(100));
        commandFrame11->AddFrame(textBox11_, new TGLayoutHints(kLHintsExpandX));

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
            std::cout << std::endl;
            std::cout << "Input root file cannot be opened." << std::endl;
            return false;
        }

        tree_ = (TTree*) file_->Get(eventTreeName_);

        if (!tree_) {
            std::cout << std::endl;
            std::cout << "Input file contains no tree \"LDMX_Events\"" << std::endl;
            return false;
        }
        eventNumMax_ = tree_->GetEntriesFast()-1;

        ecalDigiHits_ = new TClonesArray("ldmx::EcalHit");
        hcalDigiHits_ = new TClonesArray("ldmx::HcalHit");
        recoilHits_ = new TClonesArray("ldmx::SimTrackerHit");
        ecalClusters_ = new TClonesArray("ldmx::EcalCluster");
        ecalSimParticles_ = new TClonesArray("ldmx::SimTrackerHit");

        foundECALDigis_ = GetECALDigisColl(ecalDigisCollName_);
        foundHCALDigis_ = GetHCALDigisColl(hcalDigisCollName_);
        foundClusters_ = GetClustersColl(clustersCollName_);
        foundTrackerHits_ = GetTrackerHitsColl(trackerHitsCollName_);
        foundEcalSPHits_ = GetEcalSimParticlesColl(ecalSimParticlesCollName_);

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

    void EventDisplay::GetECALDigisCollInput() {

        const TString ecalDigisCollName = textBox5_->GetText();
        foundECALDigis_ = GetECALDigisColl(ecalDigisCollName);

        if (foundECALDigis_) {
            ecalDigisCollName_ = ecalDigisCollName;
            if (eventNum_ != -1) {
                GotoEvent(eventNum_);
            }
        }
    }

    bool EventDisplay::GetECALDigisColl(const TString ecalDigisCollName) {

        if (tree_->GetListOfBranches()->FindObject(ecalDigisCollName)) {
            tree_->ResetBranchAddress(tree_->GetBranch(ecalDigisCollName_));
            tree_->SetBranchAddress(ecalDigisCollName, &ecalDigiHits_);
            return true;
        } else {
            std::cout << "No branch with name \"" << ecalDigisCollName <<"\"" << std::endl;
            return false;
        }
    }

    void EventDisplay::GetHCALDigisCollInput() {

        const TString hcalDigisCollName = textBox6_->GetText();
        foundHCALDigis_ = GetHCALDigisColl(hcalDigisCollName);

        if (foundHCALDigis_) {
            hcalDigisCollName_ = hcalDigisCollName;
            if (eventNum_ != -1) {
                GotoEvent(eventNum_);
            }
        }
    }

    bool EventDisplay::GetHCALDigisColl(const TString hcalDigisCollName) {

        if (tree_->GetListOfBranches()->FindObject(hcalDigisCollName)) {
            tree_->ResetBranchAddress(tree_->GetBranch(hcalDigisCollName_));
            tree_->SetBranchAddress(hcalDigisCollName, &hcalDigiHits_);
            return true;
        } else {
            std::cout << "No branch with name \"" << hcalDigisCollName <<"\"" << std::endl;
            return false;
        }
    }

    void EventDisplay::GetTrackerHitsCollInput() {

        const TString trackerHitsCollName = textBox7_->GetText();
        trackerHitsCollName_ = trackerHitsCollName;
        foundTrackerHits_ = GetTrackerHitsColl(trackerHitsCollName);

        if (foundTrackerHits_) {
            trackerHitsCollName_ = trackerHitsCollName;
            if (eventNum_ != -1) {
                GotoEvent(eventNum_);
            }
        }
    }

    bool EventDisplay::GetTrackerHitsColl(const TString trackerHitsCollName) {
        if (tree_->GetListOfBranches()->FindObject(trackerHitsCollName)) {
            tree_->ResetBranchAddress(tree_->GetBranch(trackerHitsCollName_));
            tree_->SetBranchAddress(trackerHitsCollName, &recoilHits_);
            return true;
        } else {
            std::cout << "No branch with name \"" << trackerHitsCollName << "\"" << std::endl;
            return false;
        }
    }

    void EventDisplay::GetClustersCollInput() {

        const TString clustersCollName = textBox2_->GetText();
        foundClusters_ = GetClustersColl(clustersCollName);

        if (foundClusters_) {
            clustersCollName_ = clustersCollName;
            if (eventNum_ != -1) {
                GotoEvent(eventNum_);
            }
        }
    }

    bool EventDisplay::GetClustersColl(const TString clustersCollName) {
        if (tree_->GetListOfBranches()->FindObject(clustersCollName)) {
            tree_->ResetBranchAddress(tree_->GetBranch(clustersCollName_));
            tree_->SetBranchAddress(clustersCollName, &ecalClusters_);
            return true;
        } else {
            std::cout << "No branch with name \"" << clustersCollName << "\"" << std::endl;
            return false;
        }
    }

    void EventDisplay::GetEcalSimParticlesCollInput() {

        const TString clustersCollName = textBox2_->GetText();
        foundClusters_ = GetClustersColl(clustersCollName);

        if (foundClusters_) {
            clustersCollName_ = clustersCollName;
            if (eventNum_ != -1) {
                GotoEvent(eventNum_);
            }
        }
    }

    bool EventDisplay::GetEcalSimParticlesColl(const TString ecalSimParticlesCollName) {
        if (tree_->GetListOfBranches()->FindObject(ecalSimParticlesCollName)) {
            tree_->ResetBranchAddress(tree_->GetBranch(ecalSimParticlesCollName_));
            tree_->SetBranchAddress(ecalSimParticlesCollName, &ecalSimParticles_);
            return true;
        } else {
            std::cout << "No branch with name \"" << ecalSimParticlesCollName << "\"" << std::endl;
            return false;
        }
    }

    bool EventDisplay::GotoEvent(int event) {

        manager_->GetCurrentEvent()->DestroyElements();
        eventObjects_->Initialize();

        if (event > eventNumMax_ || event < 0) {
            std::cout << "Event number out of range." << std::endl;
            return false;
        }
        eventNum_ = event;

        printf("Loading event %d.\n", eventNum_);

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

        return true;
    }

    bool EventDisplay::GotoEvent() {

        int event = atoi(textBox1_->GetText());
        if (event == 0 && std::string(textBox1_->GetText()) != "0") {
            std::cout << "Invalid event number entered!" << std::endl;
            return false;
        }
        GotoEvent(event);
        return true;
    }

    bool EventDisplay::SetEventTree() {

        const TString treeName = textBox4_->GetText();
        TTree* tree = (TTree*) file_->Get(treeName);
        if (!tree) {
            std::cout << std::endl;
            std::cout << "Input file contains no tree \"" << treeName << "\"" << std::endl;
            return false;
        }

        tree_ = tree;
        eventTreeName_ = treeName;

        foundECALDigis_ = GetECALDigisColl(ecalDigisCollName_);
        foundHCALDigis_ = GetHCALDigisColl(hcalDigisCollName_);
        foundClusters_ = GetClustersColl(clustersCollName_);
        foundTrackerHits_ = GetTrackerHitsColl(trackerHitsCollName_);
        foundEcalSPHits_ = GetEcalSimParticlesColl(ecalSimParticlesCollName_);

        GotoEvent(eventNum_);
        return true;
    }

    bool EventDisplay::SetSimThresh() {

        double thresh = atof(textBox3_->GetText());
        if (thresh == 0 && std::string(textBox1_->GetText()) != "0") {
            std::cout << "Invalid sim energy threshold entered!" << std::endl;
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
