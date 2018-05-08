#include "EventDisplay/EventDisplay.h"

ClassImp(ldmx::EventDisplay);

// All lengths are in mm
static const double ECAL_Z_OFFSET = 214.5+290.0/2; //First number is distance from target to ECAL front face, second is half ECAL extent in z
static const double HCAL_Z_OFFSET = 504.5;
static const double HCAL_ZHIT_OFFSET = 4.5;
static const double RECOIL_SENSOR_THICKNESS = 0.52;
static const double STEREO_SEP = 3;
static const double MONO_SEP = 1;

static const double hcal_x_width = 3100.0;
static const double hcal_y_width = 3100.0;
static const double hcal_z_length = 3000.0;
static const double hcal_side_z = 290.0;
static const double hcal_top_x = 525.0;
static const double hcal_ecal_xy = 512.0;

// In radians
static const double STEREO_ANGLE = 0.1; 

namespace ldmx {

    EventDisplay::EventDisplay(TEveManager* manager) : TGMainFrame(gClient->GetRoot(), 1600, 1600) {

        manager_ = manager;
        TGLViewer* viewer = manager_->GetDefaultGLViewer();
        viewer->UseLightColorSet();

        hexReadout_ = new EcalHexReadout();
        TEveElement* ecal = drawECAL();
        TEveElement* hcal = drawHCAL();
        TEveElement* recoilTracker = drawRecoilTracker();

        detector_->AddElement(ecal);
        detector_->AddElement(hcal);
        detector_->AddElement(recoilTracker);

        manager_->AddElement(detector_);

        SetCleanup(kDeepCleanup);

        TGVerticalFrame* contents = new TGVerticalFrame(this, 500, 1200);
        TGHorizontalFrame* commandFrame1 = new TGHorizontalFrame(contents, 500,0);
        TGHorizontalFrame* commandFrame2 = new TGHorizontalFrame(contents, 500,0);
        TGHorizontalFrame* commandFrame3 = new TGHorizontalFrame(contents, 500,0);
        TGHorizontalFrame* commandFrame5 = new TGHorizontalFrame(contents, 500,0);
        TGHorizontalFrame* commandFrame6 = new TGHorizontalFrame(contents, 500,0);
        TGHorizontalFrame* commandFrame7 = new TGHorizontalFrame(contents, 500,0);
        TGHorizontalFrame* commandFrame8 = new TGHorizontalFrame(contents, 500,0);
        TGHorizontalFrame* commandFrame9 = new TGHorizontalFrame(contents, 500,0);
        TGHorizontalFrame* commandFrame10 = new TGHorizontalFrame(contents, 500,0);

        TGButton* buttonColor = new TGTextButton(commandFrame3, "Color Clusters");
        commandFrame3->AddFrame(buttonColor, new TGLayoutHints(kLHintsExpandX));
        buttonColor->Connect("Pressed()", "ldmx::EventDisplay", this, "ColorClusters()");

        TGButton* buttonPrevious = new TGTextButton(commandFrame2, "Previous Event");
        commandFrame2->AddFrame(buttonPrevious, new TGLayoutHints(kLHintsExpandX));
        buttonPrevious->Connect("Pressed()", "ldmx::EventDisplay", this, "PreviousEvent()");

        TGButton* buttonNext = new TGTextButton(commandFrame2, "Next Event");
        commandFrame2->AddFrame(buttonNext, new TGLayoutHints(kLHintsExpandX));
        buttonNext->Connect("Pressed()", "ldmx::EventDisplay", this, "NextEvent()");

        textBox_ = new TGTextEntry(commandFrame1, new TGTextBuffer(100));
        commandFrame1->AddFrame(textBox_, new TGLayoutHints(kLHintsExpandX));

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
        buttonSetTree->Connect("Pressed()", "ldmx::EventDisplay", this, "GetEventTree()");

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

        contents->AddFrame(commandFrame7, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame8, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame9, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame10, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame5, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame1, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame2, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame3, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame6, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        AddFrame(contents, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        SetWindowName("LDMX Event Display");

        MapSubwindows();
        Resize();
        MapRaised();
        MapWindow();

        manager_->FullRedraw3D(kTRUE);

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

    bool EventDisplay::GetECALDigisColl(const char* ecalDigisCollName = "ecalDigis_recon") {

        if (tree_->GetListOfBranches()->FindObject(ecalDigisCollName)) {
            tree_->SetBranchAddress(ecalDigisCollName, &ecalDigiHits_);
            return true;
        } else {
            std::cout << "No branch with name \"" << ecalDigisCollName <<"\"" << std::endl;
            return false;
        }
    }

    bool EventDisplay::GetHCALDigisColl(const char* hcalDigisCollName = "hcalDigis_recon") {

        if (tree_->GetListOfBranches()->FindObject(hcalDigisCollName)) {
            tree_->SetBranchAddress(hcalDigisCollName, &hcalDigiHits_);
            return true;
        } else {
            std::cout << "No branch with name \"" << hcalDigisCollName <<"\"" << std::endl;
            return false;
        }
    }

    bool EventDisplay::GetTrackerHitsColl(const char* trackerHitsCollName = "RecoilSimHits_sim") {
        if (tree_->GetListOfBranches()->FindObject(trackerHitsCollName)) {
            tree_->SetBranchAddress(trackerHitsCollName, &recoilHits_);
            return true;
        } else {
            std::cout << "No branch with name \"" << trackerHitsCollName << "\"" << std::endl;
            return false;
        }
    }

    bool EventDisplay::GetClustersColl(const char* clustersCollName = "ecalClusters_recon") {
        if (tree_->GetListOfBranches()->FindObject(clustersCollName)) {
            tree_->SetBranchAddress(clustersCollName, &ecalClusters_);
            return true;
        } else {
            std::cout << "No branch with name \"" << clustersCollName << "\"" << std::endl;
            return false;
        }
    }

    bool EventDisplay::GetEcalSimParticlesColl(const char* ecalSimParticlesCollName = "EcalScoringPlaneHits_sim") {
        if (tree_->GetListOfBranches()->FindObject(ecalSimParticlesCollName)) {
            tree_->SetBranchAddress(ecalSimParticlesCollName, &ecalSimParticles_);
            return true;
        } else {
            std::cout << "No branch with name \"" << ecalSimParticlesCollName << "\"" << std::endl;
            return false;
        }
    }

    bool EventDisplay::SetFile(const char* file) {

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
        foundEcalSPHits_ = GetEcalSimParticlesColl();

        return true;
    }

    bool EventDisplay::GotoEvent(int event) {

        manager_->GetCurrentEvent()->DestroyElements();

        if (event > eventNumMax_ || event < 0) {
            std::cout << "Event number out of range." << std::endl;
            return false;
        }
        eventNum_ = event;

        printf("Loading event %d.\n", eventNum_);

        hits_ = new TEveElementList("Reco Hits");
        recoObjs_ = new TEveElementList("Reco Objects");

        tree_->GetEntry(eventNum_);

        if (foundECALDigis_) {
            TEveElement* ecalHitSet = drawECALHits(ecalDigiHits_);
            hits_->AddElement(ecalHitSet);
        }

        if (foundHCALDigis_) {
            TEveElement* hcalHitSet = drawHCALHits(hcalDigiHits_);
            hits_->AddElement(hcalHitSet);
        }

        if (foundClusters_) {
            TEveElement* ecalClusterSet = drawECALClusters(ecalClusters_);
            recoObjs_->AddElement(ecalClusterSet);
        }

        if (foundTrackerHits_) {
            TEveElement* recoilHitSet = drawRecoilHits(recoilHits_);
            hits_->AddElement(recoilHitSet);
        }

        if (foundEcalSPHits_) {
            TEveElement* ecalSimParticleHitSet = drawECALSimParticles(ecalSimParticles_);
            hits_->AddElement(ecalSimParticleHitSet);
        }

        manager_->AddElement(hits_);
        manager_->AddElement(recoObjs_);
        manager_->Redraw3D(kFALSE);

        return true;
    }

    bool EventDisplay::GotoEvent() {

        int event = atoi(textBox_->GetText());
        if (event == 0 && std::string(textBox_->GetText()) != "0") {
            std::cout << "Invalid event number entered!" << std::endl;
            return false;
        }
        GotoEvent(event);
        return true;
    }

    void EventDisplay::GetClustersCollInput() {

        const char* clustersCollName = textBox2_->GetText();
        foundClusters_ = GetClustersColl(clustersCollName_);

        if (foundClusters_) {
            clustersCollName_ = clustersCollName;
            GotoEvent(eventNum_);
        }
    }

    void EventDisplay::GetECALDigisCollInput() {

        const char* ecalDigisCollName = textBox5_->GetText();
        foundECALDigis_ = GetECALDigisColl(ecalDigisCollName_);

        if (foundECALDigis_) {
            ecalDigisCollName_ = ecalDigisCollName;
            GotoEvent(eventNum_);
        }
    }

    void EventDisplay::GetHCALDigisCollInput() {

        const char* hcalDigisCollName = textBox6_->GetText();
        foundHCALDigis_ = GetHCALDigisColl(hcalDigisCollName_);

        if (foundHCALDigis_) {
            hcalDigisCollName_ = hcalDigisCollName;
            GotoEvent(eventNum_);
        }
    }

    void EventDisplay::GetTrackerHitsCollInput() {

        const char* trackerHitsCollName = textBox7_->GetText();
        trackerHitsCollName_ = trackerHitsCollName;
        foundTrackerHits_ = GetTrackerHitsColl(trackerHitsCollName_);

        if (foundTrackerHits_) {
            trackerHitsCollName_ = trackerHitsCollName;
            GotoEvent(eventNum_);
        }
    }

    bool EventDisplay::GetEventTree() {

        const char* treeName = textBox4_->GetText();
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
        foundEcalSPHits_ = GetEcalSimParticlesColl();

        GotoEvent(eventNum_);
        return true;
    }

    bool EventDisplay::SetSimThresh() {

        double thresh = atof(textBox3_->GetText());
        if (thresh == 0 && std::string(textBox_->GetText()) != "0") {
            std::cout << "Invalid sim energy threshold entered!" << std::endl;
            return false;
        }

        simThresh_ = thresh;
        TEveElement* spHits = 0;
        spHits = hits_->FindChild("ECAL SP Sim Particles");
        TEveElement::List_i sim;

        for (sim = spHits->BeginChildren(); sim != spHits->EndChildren(); sim++) {
            
            TEveElement* el = *sim;
            SimParticle* sp = (ldmx::SimParticle*)el->GetSourceObject();
            std::vector<double> pVec = sp->getMomentum();
            double p = pow(pow(pVec[0],2) + pow(pVec[1],2) + pow(pVec[2],2),0.5);
            if (p < simThresh_) { 
                el->SetRnrSelf(kFALSE); 
            } else {
                el->SetRnrSelf(kTRUE);
            }
        }

        manager_->RegisterRedraw3D();
        manager_->FullRedraw3D(kFALSE, kTRUE);

        return true;
    }

    void EventDisplay::ColorClusters() {

        TEveElement* clusters = recoObjs_->FindChild("ECAL Clusters");
        if (clusters == 0) { 
            std::cout << "No clusters to color!" << std::endl;
            return; 
        }

        int theColor = 0;
        TEveElement::List_i cluster;
        for (cluster = clusters->BeginChildren(); cluster != clusters->EndChildren(); cluster++) {
            
            TEveElement* el = *cluster;
            TEveElement::List_i hit;
            Int_t color = 0;
            if (!el->IsPickable()) {
                color = 19;
            } else if (theColor < 9) {
                color = colors_[theColor];
                theColor++;
            } else {
                Int_t ci = 200*r_.Rndm();
                color = ci;
            }

            for (hit = el->BeginChildren(); hit != el->EndChildren(); hit++) { 
                TEveElement* elChild = *hit;
                elChild->SetMainColor(color);
            }
        }

        manager_->RegisterRedraw3D();
        manager_->FullRedraw3D(kFALSE, kTRUE);
    }

    static bool compEcalHits(const EcalHit* a, const EcalHit* b) {
        return a->getEnergy() > b->getEnergy();
    }

    static bool compHcalHits(const HcalHit* a, const HcalHit* b) {
        return a->getPE() > b->getPE();
    }

    static bool compSimsP(const SimTrackerHit* a, const SimTrackerHit* b) {

        std::vector<double> paVec = a->getMomentum();
        std::vector<double> pbVec = b->getMomentum();

        double pa2 = pow(paVec[0],2)+pow(paVec[1],2)+pow(paVec[2],2);
        double pb2 = pow(pbVec[0],2)+pow(pbVec[1],2)+pow(pbVec[2],2);

        return pa2 > pb2;
    }

    static bool compSims(const SimTrackerHit* a, const SimTrackerHit* b) {

        if (a->getSimParticle() == b->getSimParticle()) {
            return compSimsP(a,b);
        } else {
            return a->getSimParticle() < b->getSimParticle();
        }
    }

    TEveStraightLineSet* EventDisplay::drawHexColumn(Double_t xCenter, Double_t yCenter, Double_t frontZ, Double_t backZ, Double_t h, Int_t color, const char* colName) {

        TEveStraightLineSet* lineset = new TEveStraightLineSet(colName);
        // Add the bins
        Double_t x[6], y[6];
        Double_t sqrt_three = sqrt(3);
        Double_t a = h / sqrt_three;
        Double_t xstart = xCenter - a;
        Double_t ystart = yCenter;

        // Go around the hexagon
        x[0] = xstart;
        y[0] = ystart;
        x[1] = x[0] + a / 2.0;
        y[1] = y[0] + a * sqrt_three / 2.0;
        x[2] = x[1] + a;
        y[2] = y[1];
        x[3] = x[2] + a / 2.0;
        y[3] = y[1] - a * sqrt_three / 2.0;
        x[4] = x[2];
        y[4] = y[3] - a * sqrt_three / 2.0;
        x[5] = x[1];
        y[5] = y[4];

        for (int nline = 0; nline < 6; ++nline) {

            int nextpt = nline+1;
            if (nline == 5) {
                nextpt = 0;
            }

            lineset->AddLine(x[nline], y[nline], frontZ, x[nextpt], y[nextpt], frontZ);
            lineset->AddLine(x[nline], y[nline], backZ, x[nextpt], y[nextpt], backZ);
            lineset->AddLine(x[nline], y[nline], frontZ, x[nline], y[nline], backZ);
        }

        lineset->SetLineColor(color);
        return lineset;
    }

    TEveBox* EventDisplay::drawBox(Float_t xPos, Float_t yPos, Float_t frontZ, Float_t xWidth, Float_t yWidth, Float_t backZ, Float_t zRotateAngle, Int_t lineColor, Int_t transparency, const char* name) {

        TEveBox *box = new TEveBox(name);

        Float_t vs[8][3] = {
                {xPos-xWidth/2,  yPos-yWidth/2,  frontZ},
                {xPos-xWidth/2,  yPos+yWidth/2,  frontZ},
                {xPos+xWidth/2,  yPos+yWidth/2,  frontZ},
                {xPos+xWidth/2,  yPos-yWidth/2,  frontZ},
                {xPos-xWidth/2,  yPos-yWidth/2,  backZ},
                {xPos-xWidth/2,  yPos+yWidth/2,  backZ},
                {xPos+xWidth/2,  yPos+yWidth/2,  backZ},
                {xPos+xWidth/2,  yPos-yWidth/2,  backZ}
        };

        Float_t rotatedvs[8][3];

        for (int m = 0; m < 8; ++m) {

            TVector3 rotatedVec = {vs[m][0],vs[m][1],vs[m][2]};
            rotatedVec.RotateZ(zRotateAngle);
            rotatedvs[m][0] = rotatedVec[0];
            rotatedvs[m][1] = rotatedVec[1];
            rotatedvs[m][2] = rotatedVec[2];
        }

        box->SetVertices(*rotatedvs);
        box->SetLineColor(lineColor);
        box->SetFillColor(lineColor);
        box->SetMainTransparency(transparency);

        return box;
    }

    TEveElement* EventDisplay::drawECAL() {

        TEveElement* ecal = new TEveElementList("ECAL");

        static const std::vector<double> xPos = {0, 0, 0, 170*sqrt(3)/2, -170*sqrt(3)/2, -170*sqrt(3)/2, 170*sqrt(3)/2};
        static const std::vector<double> yPos = {0, 170, -170, 85, 85, -85, -85};
        static const std::vector<double> zPos = {-140, 150};

        for (int col = 0; col < xPos.size(); ++col) {

            char colName[50];
            sprintf(colName, "Tower %d", col);
            TEveStraightLineSet* hexCol = drawHexColumn(xPos[col], yPos[col], zPos[0]+ECAL_Z_OFFSET, zPos[1]+ECAL_Z_OFFSET, 170, kBlue, colName);
            ecal->AddElement(hexCol);
        }

        return ecal;
    }

    TEveElement* EventDisplay::drawHCAL() {

        TEveElement* hcal = new TEveElementList("HCAL");
        TEveElement* sidehcal = new TEveElementList("Side HCAL");

        TEveBox *backHcal = drawBox(0, 0, HCAL_Z_OFFSET, hcal_x_width, hcal_y_width, HCAL_Z_OFFSET+hcal_z_length, 0, kCyan, 100, "Back HCAL");
        hcal->AddElement(backHcal);

        TEveBox *sideTopHcal = drawBox(hcal_y_width/4-hcal_ecal_xy/4, -hcal_y_width/4-hcal_ecal_xy/4, HCAL_Z_OFFSET-hcal_side_z, (hcal_y_width+hcal_ecal_xy)/2, (hcal_y_width-hcal_ecal_xy)/2, HCAL_Z_OFFSET, 0, kCyan, 100, "Module 1");
        sidehcal->AddElement(sideTopHcal);

        TEveBox *sideBottomHcal = drawBox(hcal_y_width/4+hcal_ecal_xy/4, hcal_y_width/4-hcal_ecal_xy/4, HCAL_Z_OFFSET-hcal_side_z, (hcal_y_width-hcal_ecal_xy)/2, (hcal_y_width+hcal_ecal_xy)/2, HCAL_Z_OFFSET, 0, kCyan, 100, "Module 4");
        sidehcal->AddElement(sideBottomHcal);

        TEveBox *sideLeftHcal = drawBox(-hcal_y_width/4+hcal_ecal_xy/4, hcal_y_width/4+hcal_ecal_xy/4, HCAL_Z_OFFSET-hcal_side_z, (hcal_y_width+hcal_ecal_xy)/2, (hcal_y_width-hcal_ecal_xy)/2, HCAL_Z_OFFSET, 0, kCyan, 100, "Module 2");
        sidehcal->AddElement(sideLeftHcal);

        TEveBox *sideRightHcal = drawBox(-hcal_y_width/4-hcal_ecal_xy/4, -hcal_y_width/4+hcal_ecal_xy/4, HCAL_Z_OFFSET-hcal_side_z, (hcal_y_width-hcal_ecal_xy)/2, (hcal_y_width+hcal_ecal_xy)/2, HCAL_Z_OFFSET, 0, kCyan, 100, "Module 3");
        sidehcal->AddElement(sideRightHcal);
        hcal->AddElement(sidehcal);

        //turn off HCAL by default
        hcal->SetRnrSelf(0);

        return hcal;
    }

    TEveElement* EventDisplay::drawRecoilTracker() {

        // In mm
        static const double stereo_x_width = 40.34;
        static const double stereo_y_width = 100;
        static const double mono_x_width = 50;
        static const double mono_y_width = 80;

        const std::vector<double> xPos = {-2*mono_x_width, -mono_x_width, 0, mono_x_width, 2*mono_x_width};
        const std::vector<double> yPos = {-mono_y_width/2, mono_y_width/2};
        const std::vector<double> zPos = {7.5, 22.5, 37.5, 52.5, 90.0, 180.0};

        TEveElement* recoilTracker = new TEveElementList("Recoil Tracker");
        for (int j = 0; j < 4; ++j) {

            char nfront[50];
            sprintf(nfront, "Stereo%d_front", j+1);

            char nback[50];
            sprintf(nback, "Stereo%d_back", j+1);

            TEveBox *front = drawBox(0, 0, zPos[j]-STEREO_SEP, stereo_x_width, stereo_y_width, zPos[j]-STEREO_SEP+RECOIL_SENSOR_THICKNESS, 0, kRed-10, 100, nfront);

            if (j % 2 == 0) { // Alternate angle for back layer of a stereo pair.
                TEveBox *back = drawBox(0, 0, zPos[j]+STEREO_SEP, stereo_x_width, stereo_y_width, zPos[j]+STEREO_SEP+RECOIL_SENSOR_THICKNESS, STEREO_ANGLE, kRed-10, 100, nback);
                recoilTracker->AddElement(back);
            } else {
                TEveBox *back = drawBox(0, 0, zPos[j]+STEREO_SEP, stereo_x_width, stereo_y_width, zPos[j]+STEREO_SEP+RECOIL_SENSOR_THICKNESS, -STEREO_ANGLE, kRed-10, 100, nback);
                recoilTracker->AddElement(back);
            }

            recoilTracker->AddElement(front);
        }

        int module1 = 1;
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 2; ++y) {

                char name[50];
                sprintf(name,"Mono1_%d",module1);
                ++module1;

                if (x % 2 != 0) { // Alternate mono layer z by defined separation.
                    TEveBox *front = drawBox(xPos[x], yPos[y], zPos[4]-MONO_SEP, mono_x_width, mono_y_width, zPos[4]-MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kRed-10, 100, name);
                    recoilTracker->AddElement(front);
                } else {
                    TEveBox *back = drawBox(xPos[x], yPos[y], zPos[4]+MONO_SEP, mono_x_width, mono_y_width, zPos[4]+MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kRed-10, 100, name);
                    recoilTracker->AddElement(back);
                }
            }
        }

        int module2 = 1;
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 2; ++y) {

                char name[50];
                sprintf(name,"Mono2_%d",module2);
                module2++;

                if (x % 2 != 0) { // Alternate mono layer z by defined separation.
                    TEveBox *front = drawBox(xPos[x], yPos[y], zPos[5]-MONO_SEP, mono_x_width, mono_y_width, zPos[5]-MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kRed-10, 100, name);
                    recoilTracker->AddElement(front);
                } else {
                    TEveBox *back = drawBox(xPos[x], yPos[y], zPos[5]+MONO_SEP, mono_x_width, mono_y_width, zPos[5]+MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kRed-10, 100, name);
                    recoilTracker->AddElement(back);
                }
            }
        }

        return recoilTracker;
    }

    TEveElement* EventDisplay::drawECALHits(TClonesArray* hits) {
        static const double layerZPos[] = {-137.2, -134.3, -127.95, -123.55, -115.7, -109.8, -100.7, -94.3, -85.2, -78.8, -69.7, -63.3, -54.2, -47.8, -38.7, -32.3, -23.2, -16.8, -7.7, -1.3, 7.8, 14.2, 23.3, 29.7, 42.3, 52.2, 64.8, 74.7, 87.3, 97.2, 109.8, 119.7, 132.3, 142.2};

        ldmx::EcalHit* hit;

        TEveRGBAPalette* palette = new TEveRGBAPalette(0,500.0);
        TEveElement* ecalHitSet = new TEveElementList("ECAL RecHits");

        std::vector<EcalHit*> hitVec;
        for (TIter next(hits); hit = (ldmx::EcalHit*)next();) {
            hitVec.push_back(hit);
        }

        std::sort(hitVec.begin(), hitVec.end(), compEcalHits);

        for (int i = 0; i < hitVec.size(); i++) {
            double energy = hitVec[i]->getEnergy();
            if (energy == 0) { continue; }

            unsigned int hitID = hitVec[i]->getID();
            unsigned int cellID = hitID>>15;
            unsigned int moduleID = (hitID<<17)>>29;
            int layer = hitVec[i]->getLayer();
            unsigned int combinedID = 10*cellID + moduleID;
            std::pair<double, double> xyPos = hexReadout_->getCellCenterAbsolute(combinedID);

            char digiName[50];
            sprintf(digiName, "%1.5g MeV", energy);

            const UChar_t* rgb = palette->ColorFromValue(energy);
            TColor* aColor = new TColor();
            Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);

            TEveBox *ecalDigiHit = drawBox(xyPos.first, xyPos.second, layerZPos[layer]+ECAL_Z_OFFSET-1.5, 3, 3, layerZPos[layer]+ECAL_Z_OFFSET+1.5, 0, color, 0, digiName);
            ecalHitSet->AddElement(ecalDigiHit);
        }

        ecalHitSet->SetPickableRecursively(1);

        return ecalHitSet;
    }

    TEveElement* EventDisplay::drawHCALHits(TClonesArray* hits) {

        ldmx::HcalID detID;
        ldmx::HcalHit* hit;

        TEveRGBAPalette* palette = new TEveRGBAPalette(0,100.0);
        TEveElement* hcalHitSet = new TEveElementList("HCAL RecHits");

        std::vector<HcalHit*> hitVec;
        for (TIter next(hits); hit = (ldmx::HcalHit*)next();) {
            hitVec.push_back(hit);
        }

        std::sort(hitVec.begin(), hitVec.end(), compHcalHits);

        for (int i = 0; i < hitVec.size(); i++) {
            double pe = hitVec[i]->getPE();
            if (pe == 0) { continue; }

            detID.setRawValue(hitVec[i]->getID());
            detID.unpack();

            char digiName[50];
            sprintf(digiName, "%1.5g PEs", pe);

            const UChar_t* rgb = palette->ColorFromValue(pe);
            TColor* aColor = new TColor();
            Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);

            int strip = hitVec[i]->getStrip();
            TEveBox* hcalDigiHit = 0;
            if (hitVec[i]->getSection() == 0) {
                hcalDigiHit = drawBox(hitVec[i]->getX(), 25.0+strip*50.0-750, hitVec[i]->getZ()+HCAL_ZHIT_OFFSET-5, 150, 50, hitVec[i]->getZ()+HCAL_ZHIT_OFFSET+5, 0, color, 0, digiName);
            } else if (hitVec[i]->getSection() == 1) {
                hcalDigiHit = drawBox(hitVec[i]->getX(), hitVec[i]->getY(), hitVec[i]->getZ()+HCAL_ZHIT_OFFSET-25, 150, 20, hitVec[i]->getZ()+HCAL_ZHIT_OFFSET+25, 0, color, 0, digiName);
            } else if (hitVec[i]->getSection() == 3) {
                hcalDigiHit = drawBox(hitVec[i]->getX(), hitVec[i]->getY(), hitVec[i]->getZ()+HCAL_ZHIT_OFFSET-25, 20, 150, hitVec[i]->getZ()+HCAL_ZHIT_OFFSET+25, 0, color, 0, digiName);
            } else if (hitVec[i]->getSection() == 2) {
                hcalDigiHit = drawBox(hitVec[i]->getX(), hitVec[i]->getY(), hitVec[i]->getZ()+HCAL_ZHIT_OFFSET-25, 150, 20, hitVec[i]->getZ()+HCAL_ZHIT_OFFSET+25, 0, color, 0, digiName);
            } else if (hitVec[i]->getSection() == 4) {
                hcalDigiHit = drawBox(hitVec[i]->getX(), hitVec[i]->getY(), hitVec[i]->getZ()+HCAL_ZHIT_OFFSET-25, 20, 150, hitVec[i]->getZ()+HCAL_ZHIT_OFFSET+25, 0, color, 0, digiName);
            }

            if (hcalDigiHit != 0) {
                hcalHitSet->AddElement(hcalDigiHit);
            }
        }

        hcalHitSet->SetPickableRecursively(1);

        return hcalHitSet;

    }

    TEveElement* EventDisplay::drawRecoilHits(TClonesArray* hits) {

        // In mm
        static const double stereo_strip_length = 98; // 2 mm deadspace
        static const double mono_strip_length = 78; // 2 mm deadspace

        ldmx::SimTrackerHit* hit;
        TEveElement* recoilHitSet = new TEveElementList("Recoil Sim Hits");

        for (TIter next(hits); hit = (ldmx::SimTrackerHit*)next();) {

            std::vector<float> xyzPos = hit->getPosition();

            if ((xyzPos[2] > 4 && xyzPos[2] < 5) || (xyzPos[2] > 19 && xyzPos[2] < 20) || (xyzPos[2] > 34 && xyzPos[2] < 35) || (xyzPos[2] > 49 && xyzPos[2] < 50)) {

                TEveBox *recoilHit = drawBox(xyzPos[0], 0, xyzPos[2], 1, stereo_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS, 0, kRed+1, 0, "Recoil Hit");
                recoilHitSet->AddElement(recoilHit);

            } else if ((xyzPos[2] > 10 && xyzPos[2] < 11) || (xyzPos[2] > 40 && xyzPos[2] < 41)) {

                TVector3 rotPos = {xyzPos[0], xyzPos[1], xyzPos[2]};
                rotPos.RotateZ(-STEREO_ANGLE);

                TEveBox *recoilHit = drawBox(rotPos[0], 0, xyzPos[2], 1, stereo_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS, STEREO_ANGLE, kRed+1, 0, "Recoil Hit");
                recoilHitSet->AddElement(recoilHit);

            } else if ((xyzPos[2] > 25 && xyzPos[2] < 26) || (xyzPos[2] > 55 && xyzPos[2] < 56)) {

                TVector3 rotPos = {xyzPos[0], xyzPos[1], xyzPos[2]};
                rotPos.RotateZ(STEREO_ANGLE);

                TEveBox *recoilHit = drawBox(rotPos[0], 0, xyzPos[2], 1, stereo_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS, -STEREO_ANGLE, kRed+1, 0, "Recoil Hit");
                recoilHitSet->AddElement(recoilHit);

            } else if (xyzPos[2] > 65) {
                if (fabs(xyzPos[1]) > 1.0) { // dead region

                    if (xyzPos[1] > 0) {

                        TEveBox *recoilHit = drawBox(xyzPos[0], mono_strip_length/2+1, xyzPos[2], 1, mono_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS, 0, kRed, 0, "Recoil Hit");
                        recoilHitSet->AddElement(recoilHit);
                    } else {

                        TEveBox *recoilHit = drawBox(xyzPos[0], -mono_strip_length/2-1, xyzPos[2], 1, mono_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS, 0, kRed, 0, "Recoil Hit");
                        recoilHitSet->AddElement(recoilHit);
                    }
                }
            }
        }

        return recoilHitSet;
    }

    TEveElement* EventDisplay::drawECALClusters(TClonesArray* clusters) {
        static const double layerZPos[] = {-137.2, -134.3, -127.95, -123.55, -115.7, -109.8, -100.7, -94.3, -85.2, -78.8, -69.7, -63.3, -54.2, -47.8, -38.7, -32.3, -23.2, -16.8, -7.7, -1.3, 7.8, 14.2, 23.3, 29.7, 42.3, 52.2, 64.8, 74.7, 87.3, 97.2, 109.8, 119.7, 132.3, 142.2};

        TEveRGBAPalette* palette = new TEveRGBAPalette(0,4000.0);
        TEveElement* ecalClusterSet = new TEveElementList("ECAL Clusters");

        int iC = 0;
        EcalCluster* cluster;
        for (TIter next(clusters); cluster = (ldmx::EcalCluster*)next();) {

            char clusterName[50];
            sprintf(clusterName, "ECAL Cluster %d", iC);

            TEveElement* ecalCluster = new TEveElementList(clusterName);

            double energy = cluster->getEnergy();
            std::vector<unsigned int> clusterHitIDs = cluster->getHitIDs();

            int numHits = clusterHitIDs.size();

            for (int iHit = 0; iHit < numHits; iHit++) {
                unsigned int cellID = clusterHitIDs[iHit]>>15;
                unsigned int moduleID = (clusterHitIDs[iHit]<<17)>>29;
                int layer = (clusterHitIDs[iHit]<<20)>>24;
                unsigned int combinedID = 10*cellID + moduleID;

                std::pair<double, double> xyPos = hexReadout_->getCellCenterAbsolute(combinedID);
    
                const UChar_t* rgb = palette->ColorFromValue(energy);
                TColor* aColor = new TColor();
                Int_t color = aColor->GetColor((Int_t)rgb[0], (Int_t)rgb[1], (Int_t)rgb[2]);
    
                TEveBox *ecalDigiHit = drawBox(xyPos.first, xyPos.second, layerZPos[layer]+ECAL_Z_OFFSET-1.5, 3, 3, layerZPos[layer]+ECAL_Z_OFFSET+1.5, 0, color, 0, "RecHit");
                ecalCluster->AddElement(ecalDigiHit);

                if (numHits < 2) { 
                    ecalCluster->SetPickableRecursively(0);
                } else {
                    ecalCluster->SetPickableRecursively(1);
                }
            }
            ecalClusterSet->AddElement(ecalCluster);
            iC++;
        }

        ecalClusterSet->SetPickable(1);
        return ecalClusterSet;
    }

    
    TEveElement* EventDisplay::drawECALSimParticles(TClonesArray* ecalSimParticles) {

        TEveElement* ecalSPHitSet = new TEveElementList("ECAL SP Sim Particles");

        ldmx::SimTrackerHit* ecalSPP;
        std::vector<SimTrackerHit*> simVec;
        std::vector<SimTrackerHit*> filteredSimVec;
        for (TIter next(ecalSimParticles); ecalSPP = (ldmx::SimTrackerHit*)next();) {
            simVec.push_back(ecalSPP);
        }

        std::sort(simVec.begin(), simVec.end(), compSims);
           
        SimParticle* lastP = 0; // sometimes multiple SP hits from same particle
        for (int j = 0; j < simVec.size(); j++) {
            SimParticle* sP = simVec[j]->getSimParticle();
            if (sP == lastP) continue;
            lastP = sP;
            filteredSimVec.push_back(simVec[j]);
        }

        std::sort(filteredSimVec.begin(), filteredSimVec.end(), compSimsP);

        for (int j = 0; j < filteredSimVec.size(); j++) {

            SimParticle* sP = filteredSimVec[j]->getSimParticle();

            std::vector<double> pVec = filteredSimVec[j]->getMomentum();
            std::vector<float> rVec = filteredSimVec[j]->getPosition();
            double p = pow(pow(pVec[0],2)+pow(pVec[1],2)+pow(pVec[2],2),0.5);

            double E = sP->getEnergy();

            std::vector<double> simStart = sP->getVertex();
            std::vector<double> simEnd = sP->getEndPoint();
            double rCheck = pow(pow(simEnd[0],2)+pow(simEnd[1],2)+pow(simEnd[2],2),0.5);

            double scale = 1;
            double largest = 0;
            if (abs(simEnd[0]) > 3500.0) {
                 scale = 500.0/abs(simEnd[0]-simStart[0]);
                 largest = simEnd[0];
            }
            if (abs(simEnd[1]) > 3500.0 && abs(simEnd[1]) > largest) {
                 scale = 500.0/abs(simEnd[1]-simStart[1]);
                 largest = simEnd[1];
            }
            if (abs(simEnd[2]) > 3500.0 && abs(simEnd[2]) > 3500) {
                 scale = 500.0/abs(simEnd[2]-simStart[2]);
            }

            double r = pow(pow(scale*(simEnd[0]-simStart[0]),2) + pow(scale*(simEnd[1]-simStart[1]),2) + pow(scale*(simEnd[2]-simStart[2]),2),0.5);
            signed int pdgID = sP->getPdgID();

            TEveArrow* simArr = new TEveArrow(scale*(simEnd[0]-simStart[0]),scale*(simEnd[1]-simStart[1]),scale*(simEnd[2]-simStart[2]),simStart[0],simStart[1],simStart[2]);

            simArr->SetSourceObject(sP);
            simArr->SetMainColor(kBlack);
            simArr->SetTubeR(60*0.02/r);
            simArr->SetConeL(100*0.02/r);
            simArr->SetConeR(150*0.02/r);
            simArr->SetPickable(kTRUE);
            if (p < simThresh_) { simArr->SetRnrSelf(kFALSE); }

            char name[50];
            sprintf(name, "PDG = %d, p = %1.5g MeV/c", pdgID, p);
            simArr->SetElementName(name);
            ecalSPHitSet->AddElement(simArr);
        }

        return ecalSPHitSet;
    }
}
