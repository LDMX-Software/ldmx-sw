#include "Event/TriggerResult.h"
#include "Event/EcalHit.h"
#include "Event/SimTrackerHit.h"
#include "EventDisplay/EventDisplay.h"
#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/EcalDetectorID.h"

#include "EventDisplay/EventDisplay.h"

#include "TEveBoxSet.h"
#include "TEveRGBAPalette.h"

// All lengths are in mm
static const double ECAL_Z_OFFSET = 200+510.0/2;
static const double RECOIL_SENSOR_THICKNESS = 0.52;
static const double STEREO_SEP = 3;
static const double MONO_SEP = 1;

// In radians
static const double STEREO_ANGLE = 0.1; 

EventDisplay::EventDisplay() : TGMainFrame(gClient->GetRoot(), 1000, 600) {

    TEveElement* ecal = drawECAL();
    TEveElement* recoilTracker = drawRecoilTracker();

    detector_->AddElement(ecal);
    detector_->AddElement(recoilTracker);

    gEve->AddElement(detector_);
    gEve->Redraw3D(kTRUE);

    //SetCleanup(kDeepCleanup);
    
    TGVerticalFrame* contents = new TGVerticalFrame(this, 100,200);
    TGHorizontalFrame* commandFrame1 = new TGHorizontalFrame(contents, 100,0);
    TGHorizontalFrame* commandFrame2 = new TGHorizontalFrame(contents, 100,0);
    TGHorizontalFrame* commandFrame3 = new TGHorizontalFrame(contents, 100,0);

    TGButton* buttonClose = new TGTextButton(commandFrame3, "&Exit", "gApplication->Terminate(0)");
    commandFrame3->AddFrame(buttonClose, new TGLayoutHints(kLHintsExpandX));

    TGButton* buttonPrevious = new TGTextButton(commandFrame2, "Previous Event");
    commandFrame2->AddFrame(buttonPrevious, new TGLayoutHints(kLHintsExpandX));
    buttonPrevious->Connect("Pressed()", "EventDisplay", this, "PreviousEvent()");

    TGButton* buttonNext = new TGTextButton(commandFrame2, "Next Event");
    commandFrame2->AddFrame(buttonNext, new TGLayoutHints(kLHintsExpandX));
    buttonNext->Connect("Pressed()", "EventDisplay", this, "NextEvent()");

    textBox_ = new TGTextEntry(commandFrame1, new TGTextBuffer(100));
    commandFrame1->AddFrame(textBox_, new TGLayoutHints(kLHintsExpandX));

    TGButton* buttonGoTo = new TGTextButton(commandFrame1, "Go to Event");
    commandFrame1->AddFrame(buttonGoTo, new TGLayoutHints(kLHintsExpandX));
    buttonGoTo->Connect("Pressed()", "EventDisplay", this, "GotoEvent()");

    contents->AddFrame(commandFrame1, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    contents->AddFrame(commandFrame2, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    contents->AddFrame(commandFrame3, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    AddFrame(contents, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

    SetWindowName("Event Display");

    MapSubwindows();
    Resize();
    MapRaised();
    MapWindow();
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
        eventNumMax_ = tree_->GetEntriesFast();
    
        ecalDigis_ = new TClonesArray("ldmx::EcalHit");
        recoilHits_ = new TClonesArray("ldmx::SimTrackerHit");

        return true;
}

void EventDisplay::PreviousEvent() {
    GotoEvent(eventNum_ - 1);
}

void EventDisplay::NextEvent() {
    GotoEvent(eventNum_ + 1);
}

bool EventDisplay::GotoEvent(int event) {

    gEve->GetCurrentEvent()->DestroyElements();

    if (event > eventNumMax_ || event < 0) {
        std::cout << "Event number out of range." << std::endl;
        return false;
    }
    eventNum_ = event;

    printf("Loading event %d.\n", eventNum_);

    if (hits_) {
        hits_ = new TEveElementList("Reco Hits");
        hits_->IncDenyDestroy();
    }
    else {
        hits_->DestroyElements();
    }
    tree_->GetEntry(eventNum_);
    tree_->SetBranchAddress("ecalDigis_recon", &ecalDigis_);
    tree_->SetBranchAddress("RecoilSimHits_sim", &recoilHits_);
    TEveElement* ecalHitSet = drawECALHits(ecalDigis_);
    TEveElement* recoilHitSet = drawRecoilHits(recoilHits_);
      
    hits_->AddElement(ecalHitSet);
    hits_->AddElement(recoilHitSet);
    gEve->AddElement(hits_);

    gEve->Redraw3D(kFALSE);

    return true;
}

bool EventDisplay::GotoEvent() {

    int event = atoi(textBox_->GetText());
    if (event == 0 && textBox_->GetText() != "0") {
        std::cout << "Event number must be a positive integer!" << std::endl;
        return false;
    }
    GotoEvent(event);
    return true;
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
    box->SetMainTransparency(transparency);

    return box;
}

TEveElement* EventDisplay::drawECAL() {

    TEveElement* ecal = new TEveElementList("ECAL");

    static const std::vector<double> xPos = {0, 0, 0, 170*sqrt(3)/2, -170*sqrt(3)/2, -170*sqrt(3)/2, 170*sqrt(3)/2};
    static const std::vector<double> yPos = {0, 170, -170, 85, 85, -85, -85};
    static const std::vector<double> zPos = {-252.75, 246.75};

    for (int col = 0; col < xPos.size(); ++col) {

        char colName[50];
        sprintf(colName, "Tower %d", col);
        TEveStraightLineSet* hexCol = drawHexColumn(xPos[col], yPos[col], zPos[0]+ECAL_Z_OFFSET, zPos[1]+ECAL_Z_OFFSET, 170, kCyan, colName);
        ecal->AddElement(hexCol);
    }

    return ecal;
}

TEveElement* EventDisplay::drawECALHits(TClonesArray* hits) {
    static const double layerZPos[] = {-250.5,-237.75,-225.0,-212.25,-199.5,-186.75,-174.0,-161.25,-148.5,-135.75,-123.0,-110.25,-97.5,-84.75,-72.0,-59.25,-46.5,-33.75,-21.0,-8.25,4.5,17.25,30.0,    42.75,55.5,68.25,81.0,93.75,106.5,119.25,132.0,144.75,157.5,170.25,183.0,195.75,208.5,221.25,234.0,246.75};
 
    ldmx::EcalHexReadout hex;
    ldmx::EcalDetectorID detID;

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

    for (TIter next(hits); hit = (ldmx::EcalHit*)next();) {

        double energy = hit->getEnergy();
        detID.setRawValue(hit->getID());
        detID.unpack();
        int layer = detID.getFieldValue("layer");
        std::pair<float, float> xyPos = hex.getCellCentroidXYPair(detID.getFieldValue("cell"));

        ecalHitSet->AddBox(xyPos.first, xyPos.second, layerZPos[layer-1]+ECAL_Z_OFFSET, 3, 3, 3);
        ecalHitSet->DigitValue(energy);
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
            
            TEveBox *recoilHit = drawBox(xyzPos[0], 0, xyzPos[2], 1, stereo_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS, 0, kYellow, 0, "Recoil Hit");
            recoilHitSet->AddElement(recoilHit);

        } else if ((xyzPos[2] > 10 && xyzPos[2] < 11) || (xyzPos[2] > 40 && xyzPos[2] < 41)) { 

            TVector3 rotPos = {xyzPos[0], xyzPos[1], xyzPos[2]};
            rotPos.RotateZ(-STEREO_ANGLE);

            TEveBox *recoilHit = drawBox(rotPos[0], 0, xyzPos[2], 1, stereo_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS, STEREO_ANGLE, kYellow, 0, "Recoil Hit");
            recoilHitSet->AddElement(recoilHit);

        } else if ((xyzPos[2] > 25 && xyzPos[2] < 26) || (xyzPos[2] > 55 && xyzPos[2] < 56)) {

            TVector3 rotPos = {xyzPos[0], xyzPos[1], xyzPos[2]};
            rotPos.RotateZ(STEREO_ANGLE);

            TEveBox *recoilHit = drawBox(rotPos[0], 0, xyzPos[2], 1, stereo_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS, -STEREO_ANGLE, kYellow, 0, "Recoil Hit");
            recoilHitSet->AddElement(recoilHit);

        } else if (xyzPos[2] > 65) {
	        if (fabs(xyzPos[1]) > 1.0) { // dead region

		        if (xyzPos[1] > 0) {

                    TEveBox *recoilHit = drawBox(xyzPos[0], mono_strip_length/2+1, xyzPos[2], 1, mono_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS, 0, kYellow, 0, "Recoil Hit");
                    recoilHitSet->AddElement(recoilHit);
		        } else {

                    TEveBox *recoilHit = drawBox(xyzPos[0], -mono_strip_length/2-1, xyzPos[2], 1, mono_strip_length, xyzPos[2]+RECOIL_SENSOR_THICKNESS, 0, kYellow, 0, "Recoil Hit");
                    recoilHitSet->AddElement(recoilHit);
		        }
	        }
        }
    }

    return recoilHitSet;
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

        TEveBox *front = drawBox(0, 0, zPos[j]-STEREO_SEP, stereo_x_width, stereo_y_width, zPos[j]-STEREO_SEP+RECOIL_SENSOR_THICKNESS, 0, kCyan, 100, nfront);

        if (j % 2 == 0) { // Alternate angle for back layer of a stereo pair.
            TEveBox *back = drawBox(0, 0, zPos[j]+STEREO_SEP, stereo_x_width, stereo_y_width, zPos[j]+STEREO_SEP+RECOIL_SENSOR_THICKNESS, STEREO_ANGLE, kCyan, 100, nback);
            recoilTracker->AddElement(back);
        } else {
            TEveBox *back = drawBox(0, 0, zPos[j]+STEREO_SEP, stereo_x_width, stereo_y_width, zPos[j]+STEREO_SEP+RECOIL_SENSOR_THICKNESS, -STEREO_ANGLE, kCyan, 100, nback);
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
                TEveBox *front = drawBox(xPos[x], yPos[y], zPos[4]-MONO_SEP, mono_x_width, mono_y_width, zPos[4]-MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kCyan, 100, name);
                recoilTracker->AddElement(front);
            } else {
                TEveBox *back = drawBox(xPos[x], yPos[y], zPos[4]+MONO_SEP, mono_x_width, mono_y_width, zPos[4]+MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kCyan, 100, name);
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
                TEveBox *front = drawBox(xPos[x], yPos[y], zPos[5]-MONO_SEP, mono_x_width, mono_y_width, zPos[5]-MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kCyan, 100, name);
                recoilTracker->AddElement(front);
            } else {
                TEveBox *back = drawBox(xPos[x], yPos[y], zPos[5]+MONO_SEP, mono_x_width, mono_y_width, zPos[5]+MONO_SEP+RECOIL_SENSOR_THICKNESS, 0, kCyan, 100, name);
                recoilTracker->AddElement(back);
            }
        }
    }

    return recoilTracker;
}


