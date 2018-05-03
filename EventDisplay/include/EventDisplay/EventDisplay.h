#ifndef EVENTDISPLAY_EVENTDISPLAY_H_
#define EVENTDISPLAY_EVENTDISPLAY_H_

#include "TGTextEntry.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TGLViewer.h"
#include "TGLViewer.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "TColor.h"
#include "TString.h"
#include "TRandom.h"
#include "TRint.h"

#include "TEveBox.h"
#include "TEveBrowser.h"
#include "TEveStraightLineSet.h"
#include "TEveElement.h"
#include "TEveManager.h"
#include "TEveEventManager.h"
#include "TEveArrow.h"
#include "TEveRGBAPalette.h"
#include "TEveViewer.h"

#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/HcalID.h"
#include "Event/EcalHit.h"
#include "Event/HcalHit.h"
#include "Event/SimTrackerHit.h"
#include "Event/EcalCluster.h"
#include "Event/SimParticle.h"
#include "EventDisplay/EventDisplay.h"

#include <iostream>

namespace ldmx {

    class EventDisplay : public TGMainFrame {

        public:

            EventDisplay(TEveManager* manager);

            ~EventDisplay() {
                file_->Close();
                Cleanup();
            }

            void NextEvent();

            void PreviousEvent();

            bool GetClustersColl(const char* clustersCollName);

            void GetClustersCollInput();

            bool GetECALDigisColl(const char* ecalDigisCollName);

            void GetECALDigisCollInput();

            bool GetTrackerHitsColl(const char* trackerHitsCollName);

            void GetTrackerHitsCollInput();

            bool GetEcalSimParticlesColl(const char* ecalSimParticlesCollName);

            bool GetHCALDigisColl(const char* hcalDigisCollName);

            void GetHCALDigisCollInput();

            bool GotoEvent(int event);

            bool GotoEvent();
            
            bool SetSimThresh();

            bool SetFile(const char* file);

            bool GetEventTree();

            TEveStraightLineSet* drawHexColumn(Double_t xCenter, Double_t yCenter, Double_t frontZ, Double_t backZ, Double_t h, Int_t color, const char* colName);

            TEveBox* drawBox(Float_t xPos, Float_t yPos, Float_t frontZ, Float_t xWidth, Float_t yWidth, Float_t backZ, Float_t zRotateAngle, Int_t lineColor, Int_t transparency, const char* name);

            TEveElement* drawECAL();

            TEveElement* drawHCAL();

            TEveElement* drawRecoilTracker();

            TEveElement* drawECALHits(TClonesArray* hits);

            TEveElement* drawHCALHits(TClonesArray* hits);

            TEveElement* drawRecoilHits(TClonesArray* hits);

            TEveElement* drawECALClusters(TClonesArray* clusters);

            TEveElement* drawECALSimParticles(TClonesArray* ecalSimParticles);

            void ColorClusters();

        private:

            TFile* file_;
            TTree* tree_;
            TClonesArray* ecalDigiHits_;
            TClonesArray* hcalDigiHits_;
            TClonesArray* recoilHits_;
            TClonesArray* ecalClusters_;
            TClonesArray* ecalSimParticles_;

            bool foundECALDigis_ = false;
            bool foundHCALDigis_ = false;
            bool foundClusters_ = false;
            bool foundTrackerHits_ = false;
            bool foundEcalSPHits_ = false;

            TRandom r_;
            int eventNum_ = -1;
            int eventNumMax_;
            double simThresh_ = 0;
            const char* clustersCollName_ = "ecalClusters_recon";
            const char* ecalDigisCollName_ = "ecalDigis_recon";
            const char* hcalDigisCollName_ = "hcalDigis_recon";
            const char* trackerHitsCollName_ = "RecoilSimHits_sim";
            const char* eventTreeName_ = "LDMX_Events";

            TEveElementList* hits_;
            TEveElementList* recoObjs_;
            TEveElementList* detector_ = new TEveElementList("LDMX Detector");
            TGTextEntry* textBox_;
            TGTextEntry* textBox2_;
            TGTextEntry* textBox3_;
            TGTextEntry* textBox4_;
            TGTextEntry* textBox5_;
            TGTextEntry* textBox6_;
            TGTextEntry* textBox7_;
            TGTextEntry* textBox8_;
            TGTextEntry* textBox9_;
            TGTextEntry* textBox10_;

            TEveManager* manager_{nullptr};
            std::vector<Color_t> colors_ = {kRed, kBlue, kGreen, kYellow, kMagenta, kBlack, kOrange, kPink};

            EcalHexReadout* hexReadout_{nullptr};

            ClassDef(EventDisplay, 1);
    };

}

#endif
