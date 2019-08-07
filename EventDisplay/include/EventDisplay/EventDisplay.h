#ifndef EVENTDISPLAY_EVENTDISPLAY_H_
#define EVENTDISPLAY_EVENTDISPLAY_H_

#include "TGTextEntry.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TGLViewer.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TFile.h"
#include "TString.h"
#include "TRint.h"

#include "TEveBrowser.h"
#include "TEveElement.h"
#include "TEveManager.h"
#include "TEveEventManager.h"
#include "TEveViewer.h"

#include "EventDisplay/EventObjects.h"
#include "EventDisplay/EveDetectorGeometry.h"

#include <iostream>

namespace ldmx {

    class EventDisplay : public TGMainFrame {

        public:

            EventDisplay(TEveManager* manager);

            ~EventDisplay() {

                file_->Close();
                delete file_;
                delete tree_;
                delete theDetector_;
                delete eventObjects_;
                
                delete ecalDigiHits_;
                delete hcalDigiHits_;
                delete recoilHits_;
                delete ecalClusters_;
                delete ecalSimParticles_;

                delete textBox1_;
                delete textBox2_;
                delete textBox3_;
                delete textBox4_;
                delete textBox5_;
                delete textBox6_;
                delete textBox7_;
                delete textBox8_;
                delete textBox9_;
                delete textBox10_;
                delete textBox11_;

                delete manager_;
            }

            void NextEvent();

            void PreviousEvent();

            bool GetClustersColl(const TString clustersCollName);

            void GetClustersCollInput();

            bool GetECALDigisColl(const TString ecalDigisCollName);

            void GetECALDigisCollInput();

            bool GetTrackerHitsColl(const TString trackerHitsCollName);

            void GetTrackerHitsCollInput();

            bool GetEcalSimParticlesColl(const TString ecalSimParticlesCollName);

            void GetEcalSimParticlesCollInput();

            bool GetHCALDigisColl(const TString hcalDigisCollName);

            void GetHCALDigisCollInput();

            bool GotoEvent(int event);

            bool GotoEvent();
            
            bool SetSimThresh();

            bool SetFile(const TString file);

            bool SetEventTree();

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

            int eventNum_ = -1;
            int eventNumMax_;

            TString clustersCollName_ = "ecalClusters_recon";
            TString ecalDigisCollName_ = "ecalDigis_recon";
            TString hcalDigisCollName_ = "hcalDigis_recon";
            TString trackerHitsCollName_ = "RecoilSimHits_sim";
            TString ecalSimParticlesCollName_ = "EcalScoringPlaneHits_sim";
            TString eventTreeName_ = "LDMX_Events";

            EveDetectorGeometry* theDetector_{nullptr};
            EventObjects* eventObjects_{nullptr};

            TGTextEntry* textBox1_;
            TGTextEntry* textBox2_;
            TGTextEntry* textBox3_;
            TGTextEntry* textBox4_;
            TGTextEntry* textBox5_;
            TGTextEntry* textBox6_;
            TGTextEntry* textBox7_;
            TGTextEntry* textBox8_;
            TGTextEntry* textBox9_;
            TGTextEntry* textBox10_;
            TGTextEntry* textBox11_;

            TEveManager* manager_{nullptr};

            ClassDef(EventDisplay, 1);
    };
}

#endif
