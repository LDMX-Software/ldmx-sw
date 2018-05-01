/**
 * @file NonFidEcalVetoResult.cxx
 * @brief Class used to encapsulate the results obtained from NonFidEcalVetoProcessor
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Event/NonFidEcalVetoResult.h"

ClassImp(ldmx::NonFidEcalVetoResult)

namespace ldmx {

    NonFidEcalVetoResult::NonFidEcalVetoResult() :
        TObject() {
    }

    NonFidEcalVetoResult::~NonFidEcalVetoResult() {
        Clear();
    }

    void NonFidEcalVetoResult::Clear(Option_t *option) {
        TObject::Clear();

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



        recoilPx_ = -9999;
        recoilPy_ = -9999;
        recoilPz_ = -9999;
        recoilX_ = -9999;
        recoilY_ = -9999;

        Inside_ = 0;
        FaceX_ = -9999;
        FaceY_ = -9999;

        passesVeto_.clear();
        discValue_.clear();
        ecalLayerEdepReadout_.clear();
    }

    void NonFidEcalVetoResult::Copy(TObject& object) const {

        NonFidEcalVetoResult& result = (NonFidEcalVetoResult&) object;

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

        result.discValue_ = discValue_;

        result.recoilPx_ = recoilPx_;
        result.recoilPy_ = recoilPy_;
        result.recoilPz_ = recoilPz_;
        result.recoilX_ = recoilX_;
        result.recoilY_ = recoilY_;

        result.Inside_ = Inside_;
        result.FaceX_ = FaceX_;
        result.FaceY_ = FaceY_;

        // vector copy
        result.ecalLayerEdepReadout_ = ecalLayerEdepReadout_;
    }

    void NonFidEcalVetoResult::setVariables(
            int nReadoutHits,
            int deepestLayerHit,
            int inside,

            float summedDet,
            float summedTightIso,
            float maxCellDep,
            float showerRMS,
            float xStd,
            float yStd,
            float avgLayerHit,
            float stdLayerHit,

            std::vector<float> EcalLayerEdepReadout,
            std::vector<double> recoilP,
            std::vector<float> recoilPos,
            std::vector<float> faceXY

    ) {

        nReadoutHits_ = nReadoutHits;
        summedDet_ = summedDet;
        summedTightIso_ = summedTightIso;
        Inside_ = inside;

        maxCellDep_ = maxCellDep;
        showerRMS_ = showerRMS;
        xStd_ = xStd;
        yStd_ = yStd;
        avgLayerHit_ = avgLayerHit;
        stdLayerHit_ = stdLayerHit;
        deepestLayerHit_ = deepestLayerHit;

        // discvalue not set here

        if(!recoilP.empty()){
            recoilPx_ = recoilP[0];
            recoilPy_ = recoilP[1];
            recoilPz_ = recoilP[2];
            recoilX_ = recoilPos[0];
            recoilY_ = recoilPos[1];
        }

        ecalLayerEdepReadout_ = EcalLayerEdepReadout;

        FaceX_ = faceXY[0];
        FaceY_ = faceXY[1];
    }
    void NonFidEcalVetoResult::Print(Option_t *option) const {
        std::cout << "[ NonFidEcalVetoResult ]:\n" << "\t Passes veto : " << passesVeto_[0] << "," << passesVeto_[1]
	<< "," << passesVeto_[2] << "," << passesVeto_[3] << "," << passesVeto_[4] << "\n" << std::endl;
    }
}
