// LDMX
#include "Event/TriggerResult.h"
#include "Event/EcalHit.h"
#include "Event/SimTrackerHit.h"
#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/EcalDetectorID.h"

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TObject.h"
#include "TRint.h"
#include "TColor.h"
#include "TRandom.h"
#include "TStyle.h"
#include "TMath.h"
#include "TVector3.h"
#include "TRotation.h"

// TEVE
#include "TEveManager.h"
#include "TEvePointSet.h"
#include "TEveFrameBox.h"
#include "TEveTrans.h"
#include "TEveRGBAPalette.h"
#include "TEveBox.h"
#include "TEveBoxSet.h"
#include "TEveStraightLineSet.h"

#include <string>

using namespace ldmx;

// All lengths are in mm
static const double ecalZOffset = 200+510.0/2;
static const double recoil_sensor_thickness = 0.52;
static const double stereo_sep = 3;
static const double mono_sep = 1;

// In radians
static const double stereo_angle = 0.1; 

TEveStraightLineSet* drawHexColumn(Double_t xCenter, Double_t yCenter, Double_t frontZ, Double_t backZ, Double_t h, Int_t color, const char* colName) {

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

TEveBox* drawBox(Float_t xPos, Float_t yPos, Float_t frontZ, Float_t xWidth, Float_t yWidth, Float_t backZ, Float_t zRotateAngle, Int_t lineColor, Int_t transparency, const char* name) {

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

TEveElement* drawECAL() {

    TEveElement* ecal = new TEveElementList("ECAL");

    static const std::vector<double> xPos = {0, 0, 0, 170*sqrt(3)/2, -170*sqrt(3)/2, -170*sqrt(3)/2, 170*sqrt(3)/2};
    static const std::vector<double> yPos = {0, 170, -170, 85, 85, -85, -85};
    static const std::vector<double> zPos = {-252.75, 246.75};

    for (int col = 0; col < xPos.size(); ++col) {

        char colName[50];
        sprintf(colName, "Tower %d", col);
        TEveStraightLineSet* hexCol = drawHexColumn(xPos[col], yPos[col], zPos[0]+ecalZOffset, zPos[1]+ecalZOffset, 170, kCyan, colName);
        ecal->AddElement(hexCol);
    }

    return ecal;
}

TEveElement* drawECALHits(TClonesArray* hits) {
    static const double layerZPos[] = {-250.5,-237.75,-225.0,-212.25,-199.5,-186.75,-174.0,-161.25,-148.5,-135.75,-123.0,-110.25,-97.5,-84.75,-72.0,-59.25,-46.5,-33.75,-21.0,-8.25,4.5,17.25,30.0,    42.75,55.5,68.25,81.0,93.75,106.5,119.25,132.0,144.75,157.5,170.25,183.0,195.75,208.5,221.25,234.0,246.75};
 
    EcalHexReadout hex;
    EcalDetectorID detID;

    double edepMax = 0.0;
    EcalHit* hit;

    for (TIter next(hits); hit = (EcalHit*)next();) {

        if (hit->getEnergy() > edepMax) {

            edepMax = hit->getEnergy();
        }
    }

    edepMax *= 10.0; // Scale edep by 10 to allow for smoother palette.
    TEveRGBAPalette* palette = new TEveRGBAPalette(0,edepMax);
    TEveBoxSet* ecalHitSet = new TEveBoxSet("ECAL Hits");
    ecalHitSet->SetPalette(palette);
    ecalHitSet->Reset(TEveBoxSet::kBT_AABox, kFALSE, 64);

    for (TIter next(hits); hit = (EcalHit*)next();) {

        double energy = hit->getEnergy();
        detID.setRawValue(hit->getID());
        detID.unpack();
        int layer = detID.getFieldValue("layer");
        std::pair<float, float> xyPos = hex.getCellCentroidXYPair(detID.getFieldValue("cell"));

        ecalHitSet->AddBox(xyPos.first, xyPos.second, layerZPos[layer-1]+ecalZOffset, 3, 3, 3);
        ecalHitSet->DigitValue(10.0*energy);
    }

    ecalHitSet->SetPickable(1);
    ecalHitSet->SetAlwaysSecSelect(1);
    return ecalHitSet;
}

TEveElement* drawRecoilHits(TClonesArray* hits) {

    // In mm
    static const double stereo_strip_length = 98; // 2 mm deadspace 
    static const double mono_strip_length = 78; // 2 mm deadspace

    SimTrackerHit* hit;
    TEveElement *recoilHitSet = new TEveElementList("Recoil Hits");

    for (TIter next(hits); hit = (SimTrackerHit*)next();) {

        std::vector<float> xyzPos = hit->getPosition();
        
        if ((xyzPos[2] < 5 && xyzPos[2] > 4) || (xyzPos[2] < 20 && xyzPos[2] > 19) || (xyzPos[2] < 35 && xyzPos[2] > 34) || (xyzPos[2] < 50 && xyzPos[2] > 49)) {
            
            TVector3 rotPos = {xyzPos[0], xyzPos[1], xyzPos[2]};
            rotPos.RotateZ(-stereo_angle);

            TEveBox *recoilHit = drawBox(rotPos[0], 0, xyzPos[2], 1, stereo_strip_length, xyzPos[2]+recoil_sensor_thickness, stereo_angle, kYellow, 0, "Recoil Hit");
            recoilHitSet->AddElement(recoilHit);

        } else if ((xyzPos[2] < 11 && xyzPos[2] > 10) || (xyzPos[2] < 26 && xyzPos[2] > 25) || (xyzPos[2] < 41 && xyzPos[2] > 40) || (xyzPos[2] < 56 && xyzPos[2] > 55)) { 

            TVector3 rotPos = {xyzPos[0], xyzPos[1], xyzPos[2]};
            rotPos.RotateZ(stereo_angle);

            TEveBox *recoilHit = drawBox(rotPos[0], 0, xyzPos[2], 1, stereo_strip_length, xyzPos[2]+recoil_sensor_thickness, -stereo_angle, kYellow, 0, "Recoil Hit");
            recoilHitSet->AddElement(recoilHit);

        } else if (xyzPos[2] > 65) {
	        if (fabs(xyzPos[1]) > 1.0) { // dead region

		        if (xyzPos[1] > 0) {

                    TEveBox *recoilHit = drawBox(xyzPos[0], mono_strip_length/2+1, xyzPos[2], 1, mono_strip_length, xyzPos[2]+recoil_sensor_thickness, 0, kYellow, 0, "Recoil Hit");
                    recoilHitSet->AddElement(recoilHit);
		        } else {

                    TEveBox *recoilHit = drawBox(xyzPos[0], -mono_strip_length/2-1, xyzPos[2], 1, mono_strip_length, xyzPos[2]+recoil_sensor_thickness, 0, kYellow, 0, "Recoil Hit");
                    recoilHitSet->AddElement(recoilHit);
		        }
	        }
        }
    }

    return recoilHitSet;
}

TEveElement* drawRecoilTracker() {

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

        TEveBox *front = drawBox(0, 0, zPos[j]-stereo_sep, stereo_x_width, stereo_y_width, zPos[j]-stereo_sep+recoil_sensor_thickness, stereo_angle, kCyan, 100, nfront);
        TEveBox *back = drawBox(0, 0, zPos[j]+stereo_sep, stereo_x_width, stereo_y_width, zPos[j]+stereo_sep+recoil_sensor_thickness, -stereo_angle, kCyan, 100, nback);

        recoilTracker->AddElement(front);
        recoilTracker->AddElement(back);
    }

    int module1 = 1;
    for (int x = 1; x < 4; ++x) {
        for (int y = 0; y < 2; ++y) {

            char name[50];
            sprintf(name,"Mono1_%d",module1);
            ++module1;

            if (x % 2 != 0) { // Alternate mono layer z by defined separation.
                TEveBox *front = drawBox(xPos[x], yPos[y], zPos[4]-mono_sep, mono_x_width, mono_y_width, zPos[4]-mono_sep+recoil_sensor_thickness, 0, kCyan, 100, name);
                recoilTracker->AddElement(front);
            } else {
                TEveBox *back = drawBox(xPos[x], yPos[y], zPos[4]+mono_sep, mono_x_width, mono_y_width, zPos[4]+mono_sep+recoil_sensor_thickness, 0, kCyan, 100, name);
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
                TEveBox *front = drawBox(xPos[x], yPos[y], zPos[5]-mono_sep, mono_x_width, mono_y_width, zPos[5]-mono_sep+recoil_sensor_thickness, 0, kCyan, 100, name);
                recoilTracker->AddElement(front);
            } else {
                TEveBox *back = drawBox(xPos[x], yPos[y], zPos[5]+mono_sep, mono_x_width, mono_y_width, zPos[5]+mono_sep+recoil_sensor_thickness, 0, kCyan, 100, name);
                recoilTracker->AddElement(back);
            }
        }
    }

    return recoilTracker;
}

int main(int argc, char** argv) {

    if (argc != 3) {
        std::cout << "The event display requires 2 arguments!" << std::endl;
        std::cout << "The first argument is the event number to be visualized." << std::endl;
        std::cout << "The second argument is the full path to the root file." << std::endl;
        return -1;
    }

    const char* file = argv[2];
    int iEvt = atoi(argv[1]);

    if (atoi(argv[1]) < 1 && std::string(argv[1]) != std::string("0")) {
        std::cout << "The first argument must be a positive integer." << std::endl;
        return -1;
    }
    
    TFile *infile = TFile::Open(file);

    if (!infile) {
        std::cout << std::endl;
        std::cout << "Input root file cannot be opened." << std::endl;
        return -1;
    }

    TTree *tree = (TTree*) infile->Get("LDMX_Events");

    if (!tree) {
        std::cout << std::endl;
        std::cout << "Input file contains no tree \"LDMX_Events\"" << std::endl;
        return -1;
    }

    TClonesArray *ecalDigis = new TClonesArray("ldmx::EcalHit");
    tree->SetBranchAddress("ecalDigis_digi", &ecalDigis);

    TClonesArray *recoilHits = new TClonesArray("ldmx::SimTrackerHit");
    tree->SetBranchAddress("RecoilSimHits_sim", &recoilHits);

    tree->GetEntry(iEvt);

    TRint *app = new TRint("App", &argc, argv);
    TEveManager::Create();
    
    TEveElement* ecalHitSet = drawECALHits(ecalDigis);
    TEveElement* recoilHitSet = drawRecoilHits(recoilHits);

    TEveElement* detector = new TEveElementList("Detector");
    TEveElement* ecal = drawECAL();
    TEveElement* recoilTracker = drawRecoilTracker();

    detector->AddElement(ecal);
    detector->AddElement(recoilTracker);

    gEve->AddElement(detector);
    gEve->AddElement(ecalHitSet);
    gEve->AddElement(recoilHitSet);
    gEve->Redraw3D(kTRUE);

    app->Run(kTRUE);
    app->Terminate(0);

    return 0;
} 
