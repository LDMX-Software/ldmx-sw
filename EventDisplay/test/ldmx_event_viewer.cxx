#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TObject.h"
#include "Event/TriggerResult.h"
#include "Event/EcalHit.h"
#include "Event/SimTrackerHit.h"
#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/EcalDetectorID.h"

#include "TEveManager.h"
#include "TEvePointSet.h"
#include "TEveFrameBox.h"
#include "TEveTrans.h"
#include "TEveRGBAPalette.h"
#include "TEveBox.h"
#include "TEveBoxSet.h"
#include "TEveStraightLineSet.h"
#include "TRint.h"
#include "TColor.h"
#include "TRandom.h"
#include "TStyle.h"
#include "TMath.h"

using namespace ldmx;

TEveStraightLineSet* drawHexColumn(Double_t xCenter, Double_t yCenter, Double_t frontZ, Double_t backZ, Double_t h, Int_t color) {
    static int icol=0;
    char name[100];
    sprintf(name,"Tower %d",icol); icol++;

    TEveStraightLineSet* lineset = new TEveStraightLineSet(name);
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

void visualizeECAL(TEveElement* ecal) {

    static const double zOffset = 200+510/2;

    TEveStraightLineSet* hexCol1 = drawHexColumn(0, 0, -252.75+zOffset, 246.75+zOffset, 170, kCyan);
    TEveStraightLineSet* hexCol2 = drawHexColumn(0, 170, -252.75+zOffset, 246.75+zOffset, 170, kCyan);
    TEveStraightLineSet* hexCol3 = drawHexColumn(0, -170, -252.75+zOffset, 246.75+zOffset, 170, kCyan);
    TEveStraightLineSet* hexCol4 = drawHexColumn(170*sqrt(3)/2, 85, -252.75+zOffset, 246.75+zOffset, 170, kCyan);
    TEveStraightLineSet* hexCol5 = drawHexColumn(-170*sqrt(3)/2, 85, -252.75+zOffset, 246.75+zOffset, 170, kCyan);
    TEveStraightLineSet* hexCol6 = drawHexColumn(-170*sqrt(3)/2, -85, -252.75+zOffset, 246.75+zOffset, 170, kCyan);
    TEveStraightLineSet* hexCol7 = drawHexColumn(170*sqrt(3)/2, -85, -252.75+zOffset, 246.75+zOffset, 170, kCyan);

    ecal->AddElement(hexCol1);
    ecal->AddElement(hexCol2);
    ecal->AddElement(hexCol3);
    ecal->AddElement(hexCol4);
    ecal->AddElement(hexCol5);
    ecal->AddElement(hexCol6);
    ecal->AddElement(hexCol7);

}

void visualizeTriggerRegion(TEveElement* trigReg) {

    static const double zOffset = 200+510/2;
    TEveStraightLineSet* col1 = drawHexColumn(0, 0, -252.75+zOffset, -59.25+zOffset, 170, kRed);
    TEveStraightLineSet* col2 = drawHexColumn(0, 170, -252.75+zOffset, -59.25+zOffset, 170, kRed);
    TEveStraightLineSet* col3 = drawHexColumn(0, -170, -252.75+zOffset, -59.25+zOffset, 170, kRed);
    TEveStraightLineSet* col4 = drawHexColumn(170*sqrt(3)/2, 85, -252.75+zOffset, -59.25+zOffset, 170, kRed);
    TEveStraightLineSet* col5 = drawHexColumn(-170*sqrt(3)/2, 85, -252.75+zOffset, -59.25+zOffset, 170, kRed);
    TEveStraightLineSet* col6 = drawHexColumn(-170*sqrt(3)/2, -85, -252.75+zOffset, -59.25+zOffset, 170, kRed);
    TEveStraightLineSet* col7 = drawHexColumn(170*sqrt(3)/2, -85, -252.75+zOffset, -59.25+zOffset, 170, kRed);

    trigReg->AddElement(col1);
    trigReg->AddElement(col2);
    trigReg->AddElement(col3);
    trigReg->AddElement(col4);
    trigReg->AddElement(col5);
    trigReg->AddElement(col6);
    trigReg->AddElement(col7);

}


double maxHitEdep(TTree* tree, TClonesArray* ecalDigis, int iEvt) {

    double edepMax = 0.0;
    ecalDigis->Clear();
    tree->GetEntry(iEvt);

    TIter next(ecalDigis);
    EcalHit* hit;

    while (hit = (EcalHit*)next()) {
        
        if (hit->getEnergy() > edepMax) {
            
            edepMax = hit->getEnergy();
        }
    }

    return edepMax;
}

TEveElement* visualizeECALHits(TClonesArray* hits) {
    static const double layerZPos[] = {-250.5,-237.75,-225.0,-212.25,-199.5,-186.75,-174.0,-161.25,-148.5,-135.75,-123.0,-110.25,-97.5,-84.75,-72.0,-59.25,-46.5,-33.75,-21.0,-8.25,4.5,17.25,30.0,    42.75,55.5,68.25,81.0,93.75,106.5,119.25,132.0,144.75,157.5,170.25,183.0,195.75,208.5,221.25,234.0,246.75};
 
    static const double zOffset = 200+510/2;

    EcalHexReadout hex;
    EcalDetectorID detID;

    double edepMax = 0.0;
    EcalHit* hit;

    for (TIter next(hits); hit = (EcalHit*)next();) {

        if (hit->getEnergy() > edepMax) {

            edepMax = hit->getEnergy();
        }
    }
    edepMax*=10;
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

        ecalHitSet->AddBox(xyPos.first, xyPos.second, layerZPos[layer-1]+zOffset, 3, 3, 3);
        ecalHitSet->DigitValue(10.0*energy);

    }
        //ecalHitSet->RefitPlex();

    TEveTrans& t = ecalHitSet->RefMainTrans();
    t.SetPos(0,0,0);

    ecalHitSet->SetPickable(1);
    ecalHitSet->SetAlwaysSecSelect(1);
    return ecalHitSet;
}

TEveElement* visualizeRecoilHits(TClonesArray* hits) {

    SimTrackerHit* hit;

    TEveBoxSet* recoilHitSet = new TEveBoxSet("Recoil Hits");
    recoilHitSet->Reset(TEveBoxSet::kBT_AABox, kFALSE, 64);

    for (TIter next(hits); hit = (SimTrackerHit*)next();) {

        std::vector<float> xyzPos = hit->getPosition();

        if ((xyzPos[2] < 6 && xyzPos[2] > 3) || (xyzPos[2] < 22 && xyzPos[2] > 18) || (xyzPos[2] < 40 && xyzPos[2] > 31) || (xyzPos[2] < 53 && xyzPos[2] > 41)) {
            
            recoilHitSet->AddBox(xyzPos[0], -35, xyzPos[2], 1, 60, 1);
            recoilHitSet->DigitValue(110);

        } else if ((xyzPos[2] < 12 && xyzPos[2] > 8) || (xyzPos[2] < 27 && xyzPos[2] > 23) || (xyzPos[2] < 44 && xyzPos[2] > 36) || (xyzPos[2] < 60 && xyzPos[2] > 50)) { 

            recoilHitSet->AddBox(-35, xyzPos[1], xyzPos[2], 60, 1, 1);
            recoilHitSet->DigitValue(110);

        } else if (xyzPos[2] > 65) {
         
            recoilHitSet->AddBox(-35, xyzPos[1], xyzPos[2], 60, 1, 1);
            recoilHitSet->DigitValue(110);
            
        }

    }

    return recoilHitSet;
}

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "Wrong number of arguments!" << std::endl;
    }

    const char* file = argv[2];
    int iEvt = atoi(argv[1]);

    TRint *app = new TRint("App", &argc, argv);

    TEveManager::Create();

    TFile *infile = TFile::Open(file);
    TTree *tree = (TTree*) infile->Get("LDMX_Events");

    TClonesArray *ecalDigis = new TClonesArray("ldmx::EcalHit");
    tree->SetBranchAddress("ecalDigis_digi", &ecalDigis);

    TClonesArray *recoilHits = new TClonesArray("ldmx::SimTrackerHit");
    tree->SetBranchAddress("RecoilSimHits_sim", &recoilHits);

    tree->GetEntry(iEvt);

    TEveElement* ecalHitSet = visualizeECALHits(ecalDigis);
    TEveElement* recoilHitSet = visualizeRecoilHits(recoilHits);

    TEveElement* detector = new TEveElementList("Detector");
    TEveElement* ecal = new TEveElementList("ECAL");
    visualizeECAL(ecal);

    TEveElement* trigReg = new TEveElementList("Trigger Sum Region");
    visualizeTriggerRegion(trigReg);
    
    detector->AddElement(ecal);

    gEve->AddElement(trigReg);
    gEve->AddElement(detector);
    gEve->AddElement(ecalHitSet);
    gEve->AddElement(recoilHitSet);
    gEve->Redraw3D(kTRUE);

    app->Run(kTRUE);
    app->Terminate(0);

    return 0;
} 
