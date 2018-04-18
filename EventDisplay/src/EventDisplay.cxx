#include "TEveBoxSet.h"
#include "TEveRGBAPalette.h"

#include "EventDisplay/EventDisplay.h"

ClassImp(ldmx::EventDisplay);

// All lengths are in mm
static const double ECAL_Z_OFFSET = 214.5+290.0/2; //First number is distance from target to ECAL front face, second is half ECAL extent in z
static const double RECOIL_SENSOR_THICKNESS = 0.52;
static const double STEREO_SEP = 3;
static const double MONO_SEP = 1;

// In radians
static const double STEREO_ANGLE = 0.1; 

namespace ldmx {

    EventDisplay::EventDisplay() : TGMainFrame(gClient->GetRoot(), 1600, 1200) {

        hexReadout_ = new EcalHexReadout();
        TEveElement* ecal = drawECAL();
        TEveElement* recoilTracker = drawRecoilTracker();

        detector_->AddElement(ecal);
        detector_->AddElement(recoilTracker);
        //detector_->IncDenyDestroy();

        gEve->AddElement(detector_);

        SetCleanup(kDeepCleanup);

        TGVerticalFrame* contents = new TGVerticalFrame(this, 100,200);
        TGHorizontalFrame* commandFrame1 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame2 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame3 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame4 = new TGHorizontalFrame(contents, 100,0);
        TGHorizontalFrame* commandFrame5 = new TGHorizontalFrame(contents, 100,0);

        TGButton* buttonColor = new TGTextButton(commandFrame3, "Color Clusters");
        commandFrame3->AddFrame(buttonColor, new TGLayoutHints(kLHintsExpandX));
        buttonColor->Connect("Pressed()", "ldmx::EventDisplay", this, "ColorClusters()");

        TGButton* buttonClose = new TGTextButton(commandFrame4, "&Exit", "gApplication->Terminate(0)");
        commandFrame4->AddFrame(buttonClose, new TGLayoutHints(kLHintsExpandX));

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

        TGButton* buttonClusterName = new TGTextButton(commandFrame5, "Cluster Collection Name");
        commandFrame5->AddFrame(buttonClusterName, new TGLayoutHints(kLHintsExpandX));
        buttonClusterName->Connect("Pressed()", "ldmx::EventDisplay", this, "GetClustersCollInput()");

        contents->AddFrame(commandFrame1, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame2, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame3, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame5, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
        contents->AddFrame(commandFrame4, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        AddFrame(contents, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

        SetWindowName("LDMX Event Display");

        MapSubwindows();
        Resize();
        MapRaised();
        MapWindow();

        gEve->FullRedraw3D(kTRUE);

    }

    void EventDisplay::PreviousEvent() {
        if (eventNum_ == 0) {
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

    bool EventDisplay::GetEcalSPHitsColl(const char* ecalSPHitsCollName = "EcalScoringPlaneHits_sim") {
        if (tree_->GetListOfBranches()->FindObject(ecalSPHitsCollName)) {
            tree_->SetBranchAddress(ecalSPHitsCollName, &ecalSPHits_);
            return true;
        } else {
            std::cout << "No branch with name \"" << ecalSPHitsCollName << "\"" << std::endl;
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

        tree_ = (TTree*) file_->Get("LDMX_Events");

        if (!tree_) {
            std::cout << std::endl;
            std::cout << "Input file contains no tree \"LDMX_Events\"" << std::endl;
            return false;
        }
        eventNumMax_ = tree_->GetEntriesFast()-1;

        ecalDigiHits_ = new TClonesArray("ldmx::EcalHit");
        recoilHits_ = new TClonesArray("ldmx::SimTrackerHit");
        ecalClusters_ = new TClonesArray("ldmx::EcalCluster");
        ecalSPHits_ = new TClonesArray("ldmx::SimTrackerHit");

        foundECALDigis_ = GetECALDigisColl();
        foundClusters_ = GetClustersColl(clustersCollName_);
        foundTrackerHits_ = GetTrackerHitsColl();
        foundEcalSPHits_ = GetEcalSPHitsColl();

        return true;
    }

    bool EventDisplay::GotoEvent(int event) {

        gEve->GetCurrentEvent()->DestroyElements();

        if (event > eventNumMax_ || event < 0) {
            std::cout << "Event number out of range." << std::endl;
            return false;
        }
        eventNum_ = event;

        printf("Loading event %d.\n", eventNum_);

        hits_ = new TEveElementList("Reco Hits");
        recoObjs_ = new TEveElementList("Reco Objects");
        simHits_ = new TEveElementList("Sim Particles");

        tree_->GetEntry(eventNum_);

        if (foundECALDigis_) {
            TEveElement* ecalHitSet = drawECALHits(ecalDigiHits_);
            hits_->AddElement(ecalHitSet);
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
            TEveElement* ecalSPHitSet = drawECALSPHits(ecalSPHits_);
            hits_->AddElement(ecalSPHitSet);
        }

        gEve->AddElement(hits_);
        gEve->AddElement(recoObjs_);
        gEve->Redraw3D(kFALSE);

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
        clustersCollName_ = clustersCollName;
        foundClusters_ = GetClustersColl(clustersCollName_);
        GotoEvent(eventNum_);
    }

    void EventDisplay::ColorClusters() {

        std::cout << "Making pretty clusters!" << std::endl;

        TEveElement* clusters = recoObjs_->FindChild("ECAL Clusters");
        if (clusters == 0) { 
            std::cout << "No clusters to color!" << std::endl;
            return; 
        }

        int theColor = 0;
        for (int iC = 0; iC < nclusters_; iC++) {
            
            char clusterName[50];
            sprintf(clusterName, "ECAL Cluster %d", iC);
            TEveBoxSet* aCluster = (TEveBoxSet*)clusters->FindChild(clusterName);
            aCluster->UseSingleColor();

            if (aCluster->GetDefDepth() == -1) {
                aCluster->SetMainColor(19);
            } else if (theColor < 9) {
                aCluster->SetMainColor(colors_[theColor]);
                theColor++;
            } else {
                Int_t ci = 200*r_.Rndm();
                aCluster->SetMainColor(ci);
            }

        }

        gEve->RegisterRedraw3D();
        gEve->FullRedraw3D(kFALSE, kTRUE);
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
                {xPos+xWidth/2,  yPos-yWidth/2,  frontZ},
                {xPos+xWidth/2,  yPos+yWidth/2,  frontZ},
                {xPos-xWidth/2,  yPos+yWidth/2,  frontZ},
                {xPos-xWidth/2,  yPos+yWidth/2,  backZ},
                {xPos+xWidth/2,  yPos+yWidth/2,  backZ},
                {xPos+xWidth/2,  yPos-yWidth/2,  backZ},
                {xPos-xWidth/2,  yPos-yWidth/2,  backZ}
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

    TEveElement* EventDisplay::drawECALHits(TClonesArray* hits) {
        static const double layerZPos[] = {-137.2, -134.3, -127.95, -123.55, -115.7, -109.8, -100.7, -94.3, -85.2, -78.8, -69.7, -63.3, -54.2, -47.8, -38.7, -32.3, -23.2, -16.8, -7.7, -1.3, 7.8, 14.2, 23.3, 29.7, 42.3, 52.2, 64.8, 74.7, 87.3, 97.2, 109.8, 119.7, 132.3, 142.2};

        double edepMax = 0.0;
        ldmx::EcalHit* hit;

        for (TIter next(hits); hit = (ldmx::EcalHit*)next();) {

            if (hit->getEnergy() > edepMax) {

                edepMax = hit->getEnergy();
            }
        }

        TEveRGBAPalette* palette = new TEveRGBAPalette(0,edepMax);
        TEveBoxSet* ecalHitSet = new TEveBoxSet("ECAL Hits");
        ecalHitSet->SetPalette(palette);
        ecalHitSet->Reset(TEveBoxSet::kBT_AABox, kFALSE, 64);

        int i = 0;
        for (TIter next(hits); hit = (ldmx::EcalHit*)next();) {

            double energy = hit->getEnergy();
            if (energy == 0) { continue; }

            unsigned int hitID = hit->getID();
            unsigned int cellID = hitID>>15;
            unsigned int moduleID = (hitID<<17)>>29;
            int layer = hit->getLayer();
            unsigned int combinedID = 10*cellID + moduleID;
            std::pair<double, double> xyPos = hexReadout_->getCellCenterAbsolute(combinedID);

            ecalHitSet->AddBox(xyPos.first, xyPos.second, layerZPos[layer]+ECAL_Z_OFFSET, 3, 3, 3);
            ecalHitSet->DigitValue(energy);
            i++;
        }

        ecalHitSet->SetPickable(1);
        ecalHitSet->SetAlwaysSecSelect(1);
        return ecalHitSet;
    }

    TEveElement* EventDisplay::drawRecoilHits(TClonesArray* hits) {

        // In mm
        static const double stereo_strip_length = 98; // 2 mm deadspace
        static const double mono_strip_length = 78; // 2 mm deadspace

        ldmx::SimTrackerHit* hit;
        TEveElement* recoilHitSet = new TEveElementList("Recoil Hits");

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

                        TEveBox *recoilHit = drawBox(xyzPos[0], mono_strip_length/2+1, xyzPos[2], 1, mono_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS+1, 0, kRed, 0, "Recoil Hit");
                        recoilHitSet->AddElement(recoilHit);
                    } else {

                        TEveBox *recoilHit = drawBox(xyzPos[0], -mono_strip_length/2-1, xyzPos[2], 1, mono_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS+1, 0, kRed, 0, "Recoil Hit");
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
        int numC = 0;
        EcalCluster* cluster;
        for (TIter next(clusters); cluster = (ldmx::EcalCluster*)next();) {

            char clusterName[50];
            sprintf(clusterName, "ECAL Cluster %d", iC);

            TEveBoxSet* ecalCluster = new TEveBoxSet(clusterName);
            ecalCluster->SetPalette(palette);
            ecalCluster->Reset(TEveBoxSet::kBT_AABox, kFALSE, 64);

            double energy = cluster->getEnergy();
            std::vector<unsigned int> clusterHitIDs = cluster->getHitIDs();

            int numHits = clusterHitIDs.size();

            // Use sketchy TEveBoxSet method to set property of one-hit clusters
            // that is to be used when coloring clusters later on.
            if (numHits < 2) { 
                ecalCluster->SetDefDepth(-1); 
            } else {
                numC++;
            }
            for (int iHit = 0; iHit < clusterHitIDs.size(); iHit++) {
                unsigned int cellID = clusterHitIDs[iHit]>>15;
                unsigned int moduleID = (clusterHitIDs[iHit]<<17)>>29;
                int layer = (clusterHitIDs[iHit]<<20)>>24;
                unsigned int combinedID = 10*cellID + moduleID;

                std::pair<double, double> xyPos = hexReadout_->getCellCenterAbsolute(combinedID);

                ecalCluster->AddBox(xyPos.first, xyPos.second, layerZPos[layer]+ECAL_Z_OFFSET, 3, 3, 3);
                ecalCluster->DigitValue(energy);

                ecalCluster->SetAlwaysSecSelect(1);

            }
            ecalClusterSet->AddElement(ecalCluster);
            iC++;
        }
        nclusters_ = iC;
        std::cout << "Total number of clusters: " << numC << std::endl;

        ecalClusterSet->SetPickable(1);
        return ecalClusterSet;
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
    
    TEveElement* EventDisplay::drawECALSPHits(TClonesArray* ecalSPHits) {

        TEveElement* ecalSPHitSet = new TEveElementList("ECAL Scoring Plane Hits");

        ldmx::SimTrackerHit* ecalSPP;
        for (TIter next(ecalSPHits); ecalSPP = (ldmx::SimTrackerHit*)next();) {

            std::vector<float> rVec = ecalSPP->getPosition();
            // For now skip hits not on front scoring plane
            if (rVec[2] > 220) { continue; }

            std::vector<double> pVec = ecalSPP->getMomentum();
            double p = pow(pow(pVec[0],2)+pow(pVec[1],2)+pow(pVec[2],2),0.5);

            SimParticle* sP = ecalSPP->getSimParticle();
            signed int pdgID = sP->getPdgID();

            //Do some interesting stuff here
            TEveArrow* track = new TEveArrow(25*pVec[0]/p,25*pVec[1]/p,25*pVec[2]/p,rVec[0],rVec[1],rVec[2]);
            track->SetMainColor(kBlack);
            track->SetTubeR(0.02);
            track->SetPickable(kTRUE);

            char name[50];
            sprintf(name, "PDG = %d, P = %1.5g MeV", pdgID,p);
            track->SetElementName(name);
            ecalSPHitSet->AddElement(track);
        }

        return ecalSPHitSet;
    }
}
