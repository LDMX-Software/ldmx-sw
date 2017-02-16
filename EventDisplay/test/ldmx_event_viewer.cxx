#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TCanvas.h"
#include "TClonesArray.h"
#include "TObject.h"
#include "Event/TriggerResult.h"
#include "Event/EcalHit.h"
#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/EcalDetectorID.h"

#include "TEveManager.h"
#include "TEvePointSet.h"
#include "TEveFrameBox.h"
#include "TEveTrans.h"
#include "TEveRGBAPalette.h"
#include "TEveBox.h"
#include "TEveBoxSet.h"
#include "TRint.h"
#include "TColor.h"
#include "TRandom.h"
#include "TStyle.h"
#include "TMath.h"

using namespace ldmx;

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

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "Wrong number of arguments!" << std::endl;
    }
    std::vector<double> layerZPos = {-250.5,-237.75,-225.0,-212.25,-199.5,-186.75,-174.0,-161.25,-148.5,-135.75,-123.0,-110.25,-97.5,-84.75,-72.0,-59.25,-46.5,-33.75,-21.0,-8.25,4.5,17.25,30.0,42.75,55.5,68.25,81.0,93.75,106.5,119.25,132.0,144.75,157.5,170.25,183.0,195.75,208.5,221.25,234.0,246.75};

    const char* file = argv[2];
    int iEvt = atoi(argv[1]);

    TRint *app = new TRint("App", &argc, argv);

    TEveManager::Create();

    EcalHexReadout* det = new EcalHexReadout();
    EcalDetectorID* detID = new EcalDetectorID();

    TFile *infile = TFile::Open(file);
    TTree *tree = (TTree*) infile->Get("LDMX_Events");

    TClonesArray *ecalDigis = new TClonesArray("ldmx::EcalHit");
    tree->SetBranchAddress("ecalDigis_digi", &ecalDigis);

    double maxEdep = maxHitEdep(tree, ecalDigis, iEvt);
    TEveRGBAPalette* palette = new TEveRGBAPalette(0,maxEdep);

    TEveBoxSet* boxset = new TEveBoxSet("BoxSet");
    boxset->SetPalette(palette);
    boxset->Reset(TEveBoxSet::kBT_AABox, kFALSE, 64);

    tree->GetEntry(iEvt);

    TIter next(ecalDigis);
    EcalHit* hit;

    while (hit = (EcalHit*)next()) {

        int detIDraw = hit->getID();
        double energy = hit->getEnergy();
        detID->setRawValue(detIDraw);
        detID->unpack();
        int cellid = detID->getFieldValue("cell");
        int layer = detID->getFieldValue("layer");
        std::pair<float, float> xyPos = det->getCellCentroidXYPair(cellid);

        boxset->AddBox(xyPos.first, xyPos.second, layerZPos[layer-1], 3, 3, 3);
        boxset->DigitValue(energy);

    }
    boxset->RefitPlex();
 
    TEveTrans& t = boxset->RefMainTrans();
    t.SetPos(0,0,0);

    boxset->SetPickable(1);
    boxset->SetAlwaysSecSelect(1);

    gEve->AddElement(boxset);
    gEve->Redraw3D(kTRUE);

    TEveBox* b = new TEveBox();
    b->SetMainColor(kCyan);
    b->SetMainTransparency(100);
    
    b->SetVertex(0, -255, 255, -255);
    b->SetVertex(1, -255, -255, -255);
    b->SetVertex(2, 255, -255, -255);
    b->SetVertex(3, 255, 255, -255);
    b->SetVertex(4, -255, 255, 252.75);
    b->SetVertex(5, -255, -255, 252.75);
    b->SetVertex(6, 255, -255, 252.75);
    b->SetVertex(7, 255, 255, 252.75);
    
    gEve->AddElement(b);

    app->Run(kTRUE);
    app->Terminate(0);

    return 0;
} 
