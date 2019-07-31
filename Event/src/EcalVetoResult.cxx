/**
 * @file EcalVetoResult.cxx
 * @brief Class used to encapsulate the results obtained from EcalVetoProcessor
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/EcalVetoResult.h"

ClassImp(ldmx::EcalVetoResult)

namespace ldmx {

    EcalVetoResult::EcalVetoResult() :
        TObject() {
    }

    EcalVetoResult::~EcalVetoResult() {
        Clear();
    }

    void EcalVetoResult::Clear(Option_t *option) {
        TObject::Clear();

        passesVeto_ = false;

        nReadoutHits_ = 0;
        summedDet_ = 0;
        summedTightIso_ = 0;
        maxCellDep_ = 0;
        showerRMS_ = 0;
        xStd_ = 0;
        yStd_ = 0;
        avgLayerHit_ = 0;
        stdLayerHit_ = 0;
        deepestLayerHit_ = 0;
        ecalBackEnergy_ = 0;

        electronContainmentEnergy_.clear();
        photonContainmentEnergy_.clear();
        outsideContainmentEnergy_.clear();
        outsideContainmentNHits_.clear();
        outsideContainmentXStd_.clear();
        outsideContainmentYStd_.clear();
        
        discValue_ = 0;

        recoilPx_ = -9999;
        recoilPy_ = -9999;
        recoilPz_ = -9999;
        recoilX_ = -9999; 
        recoilY_ = -9999;

        ecalLayerEdepReadout_.clear();
    }

    void EcalVetoResult::Copy(TObject& object) const {

        EcalVetoResult& result = (EcalVetoResult&) object;

        result.passesVeto_ = passesVeto_;

        result.nReadoutHits_ = nReadoutHits_;
        result.summedDet_ = summedDet_;
        result.summedTightIso_ = summedTightIso_;
        result.maxCellDep_ = maxCellDep_;
        result.showerRMS_ = showerRMS_;
        result.xStd_ = xStd_;
        result.yStd_ = yStd_;
        result.avgLayerHit_ = avgLayerHit_;
        result.stdLayerHit_ = stdLayerHit_;
        result.deepestLayerHit_ = deepestLayerHit_;
        result.ecalBackEnergy_ = ecalBackEnergy_;
        
        result.electronContainmentEnergy_ = electronContainmentEnergy_;
        result.photonContainmentEnergy_ = photonContainmentEnergy_;
        result.outsideContainmentEnergy_ = outsideContainmentEnergy_;
        result.outsideContainmentNHits_ = outsideContainmentNHits_;
        result.outsideContainmentXStd_ = outsideContainmentXStd_;
        result.outsideContainmentYStd_ = outsideContainmentYStd_;
        
        result.discValue_ = discValue_;
        
        result.recoilPx_ = recoilPx_;
        result.recoilPy_ = recoilPy_;
        result.recoilPz_ = recoilPz_;
        result.recoilX_ = recoilX_; 
        result.recoilY_ = recoilY_;

        // vector copy
        result.ecalLayerEdepReadout_ = ecalLayerEdepReadout_;
    }

    void EcalVetoResult::setVariables(
            int nReadoutHits,
            int deepestLayerHit,
            float summedDet,
            float summedTightIso,
            float maxCellDep,
            float showerRMS,
            float xStd,
            float yStd,
            float avgLayerHit,
            float stdLayerHit,
            float ecalBackEnergy,
            
            std::vector<float> electronContainmentEnergy,
            std::vector<float> photonContainmentEnergy,
            std::vector<float> outsideContainmentEnergy,
            std::vector<int>   outsideContainmentNHits,
            std::vector<float> outsideContainmentXStd,
            std::vector<float> outsideContainmentYStd,
            
            std::vector<float> EcalLayerEdepReadout,
            std::vector<double> recoilP, 
            std::vector<float> recoilPos
    ) { 

        nReadoutHits_ = nReadoutHits;
        summedDet_ = summedDet;
        summedTightIso_ = summedTightIso;
        maxCellDep_ = maxCellDep;
        showerRMS_ = showerRMS;
        xStd_ = xStd;
        yStd_ = yStd;
        avgLayerHit_ = avgLayerHit;
        stdLayerHit_ = stdLayerHit;
        deepestLayerHit_ = deepestLayerHit;
        ecalBackEnergy_ = ecalBackEnergy;

        electronContainmentEnergy_ = electronContainmentEnergy;
        photonContainmentEnergy_ = photonContainmentEnergy;
        outsideContainmentEnergy_ = outsideContainmentEnergy;
        outsideContainmentNHits_ = outsideContainmentNHits;
        outsideContainmentXStd_ = outsideContainmentXStd;
        outsideContainmentYStd_ = outsideContainmentYStd;
        
        // discvalue not set here

        if(!recoilP.empty()){
            recoilPx_ = recoilP[0]; 
            recoilPy_ = recoilP[1]; 
            recoilPz_ = recoilP[2];
            recoilX_ = recoilPos[0]; 
            recoilY_ = recoilPos[1]; 
        }

        ecalLayerEdepReadout_ = EcalLayerEdepReadout;
    }

    void EcalVetoResult::Print(Option_t *option) const {
        std::cout << "[ EcalVetoResult ]:\n" << "\t Passes veto : " << passesVeto_ << "\n" << std::endl;
    }
}
