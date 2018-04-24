#ifndef EVENTDISPLAY_EVENTDISPLAY_H_
#define EVENTDISPLAY_EVENTDISPLAY_H_

#include "TGTextEntry.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "TColor.h"
#include "TString.h"
#include "TRandom.h"

#include "TEveBox.h"
#include "TEveStraightLineSet.h"
#include "TEveElement.h"
#include "TEveManager.h"
#include "TEveEventManager.h"
#include "TEveArrow.h"

#include "DetDescr/EcalHexReadout.h"
#include "Event/TriggerResult.h"
#include "Event/EcalHit.h"
#include "Event/SimTrackerHit.h"
#include "Event/EcalCluster.h"
#include "Event/SimParticle.h"
#include "EventDisplay/EventDisplay.h"

#include <iostream>

namespace ldmx {

    class EventDisplay : public TGMainFrame {

        public:

            EventDisplay();

            ~EventDisplay() {
                file_->Close();
                Cleanup();
            }

            void NextEvent();

            void PreviousEvent();

            bool GetClustersColl(const char* clustersCollName);

            void GetClustersCollInput();

            bool GetECALDigisColl(const char* ecalDigisCollName);

            bool GetTrackerHitsColl(const char* trackerHitsCollName);

            bool GetEcalSimParticlesColl(const char* ecalSimParticlesCollName);

            bool GotoEvent(int event);

            bool GotoEvent();
            
            bool SetSimThresh();

            bool SetFile(const char* file);

            TEveStraightLineSet* drawHexColumn(Double_t xCenter, Double_t yCenter, Double_t frontZ, Double_t backZ, Double_t h, Int_t color, const char* colName);

            TEveBox* drawBox(Float_t xPos, Float_t yPos, Float_t frontZ, Float_t xWidth, Float_t yWidth, Float_t backZ, Float_t zRotateAngle, Int_t lineColor, Int_t transparency, const char* name);

            TEveElement* drawECAL();

            TEveElement* drawRecoilTracker();

            TEveElement* drawECALHits(TClonesArray* hits);

            TEveElement* drawRecoilHits(TClonesArray* hits);

            TEveElement* drawECALClusters(TClonesArray* clusters);

            TEveElement* drawECALSimParticles(TClonesArray* ecalSimParticles);

            void ColorClusters();

        private:

            TFile* file_;
            TTree* tree_;
            TClonesArray* ecalDigiHits_;
            TClonesArray* recoilHits_;
            TClonesArray* ecalClusters_;
            TClonesArray* ecalSimParticles_;

            bool foundECALDigis_ = false;
            bool foundClusters_ = false;
            bool foundTrackerHits_ = false;
            bool foundEcalSPHits_ = false;

            TRandom r_;
            int eventNum_ = 0;
            int eventNumMax_;
            double simThresh_ = 0;
            const char* clustersCollName_ = "ecalClusters_recon";
            TEveElementList* hits_;
            TEveElementList* recoObjs_;
            TEveElementList* detector_ = new TEveElementList("LDMX Detector");
            TGTextEntry* textBox_;
            TGTextEntry* textBox2_;
            TGTextEntry* textBox3_;

            std::vector<Color_t> colors_ = {kRed, kBlue, kGreen, kYellow, kMagenta, kBlack, kOrange, kPink};

            EcalHexReadout* hexReadout_{nullptr};

            ClassDef(EventDisplay, 1);
    };

}

#endif
