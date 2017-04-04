#ifndef EVENTDISPLAY_EVENTDISPLAY_H_
#define EVENTDISPLAY_EVENTDISPLAY_H_

#include "TGTextEntry.h"
#include "TGFrame.h"
#include "TGButton.h"
#include "TTree.h"
#include "TClonesArray.h"
#include "TFile.h"

#include "TEveBox.h"
#include "TEveStraightLineSet.h"
#include "TEveElement.h"
#include "TEveManager.h"
#include "TEveEventManager.h"

#include <iostream>

class EventDisplay : public TGMainFrame {

    public:

        EventDisplay();
        ~EventDisplay() {
            gEve->GetCurrentEvent()->DestroyElements();
            file_->Close();
            delete detector_;
            Cleanup();
        }

        void NextEvent();
        void PreviousEvent();
        bool GotoEvent(int event);
        bool GotoEvent();
        bool SetFile(const char* file);

        TEveStraightLineSet* drawHexColumn(Double_t xCenter, Double_t yCenter, Double_t frontZ, Double_t backZ, Double_t h, Int_t color, const char* colName);
        TEveBox* drawBox(Float_t xPos, Float_t yPos, Float_t frontZ, Float_t xWidth, Float_t yWidth, Float_t backZ, Float_t zRotateAngle, Int_t lineColor, Int_t transparency, const char* name);
        TEveElement* drawECAL();
        TEveElement* drawRecoilTracker();
        TEveElement* drawECALHits(TClonesArray* hits);
        TEveElement* drawRecoilHits(TClonesArray* hits);
        
    private:

        TFile* file_;
        TTree* tree_;
        TClonesArray* ecalDigis_;
        TClonesArray* recoilHits_;

        int eventNum_ = 0;
        int eventNumMax_;
        TEveElementList* hits_;
        TEveElementList* detector_ = new TEveElementList("Detector");
        TGTextEntry* textBox_;

};

#endif
