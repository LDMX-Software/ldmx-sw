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
        discValue_ = 0;
        nReadoutHits_ = 0;
        nLooseIsoHits_ = 0;
        nTightIsoHits_ = 0;
        summedDet_ = 0;
        summedOuter_ = 0;
        backSummedDet_ = 0;
        summedLooseIso_ = 0;
        maxLooseIsoDep_ = 0;
        summedTightIso_ = 0;
        maxTightIsoDep_ = 0;
        maxCellDep_ = 0;
        showerRMS_ = 0;
        nLooseMipTracks_ = 0;
        nMediumMipTracks_ = 0;
        nTightMipTracks_ = 0;

        ecalLayerEdepReadout_.clear();
        looseMipTracks_.clear();
        mediumMipTracks_.clear();
        tightMipTracks_.clear();
    }

    void EcalVetoResult::Copy(TObject& object) const {

        EcalVetoResult& result = (EcalVetoResult&) object;
        result.passesVeto_ = passesVeto_;
        result.discValue_ = discValue_;
        result.nReadoutHits_ = nReadoutHits_;
        result.nLooseIsoHits_ = nLooseIsoHits_;
        result.nTightIsoHits_ = nTightIsoHits_;
        result.summedDet_ = summedDet_;
        result.summedOuter_ = summedOuter_;
        result.backSummedDet_ = backSummedDet_;
        result.summedLooseIso_ = summedLooseIso_;
        result.maxLooseIsoDep_ = maxLooseIsoDep_;
        result.summedTightIso_ = summedTightIso_;
        result.maxTightIsoDep_ = maxTightIsoDep_;
        result.maxCellDep_ = maxCellDep_;
        result.showerRMS_ = showerRMS_;
        result.ecalLayerEdepReadout_ = ecalLayerEdepReadout_;
        result.looseMipTracks_ = looseMipTracks_;
        result.mediumMipTracks_ = mediumMipTracks_;
        result.tightMipTracks_ = tightMipTracks_;
        result.nLooseMipTracks_ = nLooseMipTracks_;
        result.nMediumMipTracks_ = nMediumMipTracks_;
        result.nTightMipTracks_ = nTightMipTracks_;
    }

    void EcalVetoResult::setVariables(int nReadoutHits, int nLooseIsoHits, int nTightIsoHits, float summedDet,
            int summedOuter, float backSummedDet, float summedLooseIso, float maxLooseIsoDep, float summedTightIso,
            float maxTightIsoDep, float maxCellDep, float showerRMS, std::vector<float> EcalLayerEdepReadout,
            std::vector<std::pair<int, float>> looseMipTracks, std::vector<std::pair<int, float>> mediumMipTracks,
            std::vector<std::pair<int, float>> tightMipTracks) {

        nReadoutHits_ = nReadoutHits;
        nLooseIsoHits_ = nLooseIsoHits;
        nTightIsoHits_ = nTightIsoHits;
        summedDet_ = summedDet;
        summedOuter_ = summedOuter;
        backSummedDet_ = backSummedDet;
        summedLooseIso_ = summedLooseIso;
        maxLooseIsoDep_ = maxLooseIsoDep;
        summedTightIso_ = summedTightIso;
        maxTightIsoDep_ = maxTightIsoDep;
        maxCellDep_ = maxCellDep;
        showerRMS_ = showerRMS;

        ecalLayerEdepReadout_ = EcalLayerEdepReadout;
        looseMipTracks_ = looseMipTracks;
        mediumMipTracks_ = mediumMipTracks;
        tightMipTracks_ = tightMipTracks;

        nLooseMipTracks_ = looseMipTracks.size();
        nMediumMipTracks_ = mediumMipTracks.size();
        nTightMipTracks_ = tightMipTracks.size();
    }

    void EcalVetoResult::Print(Option_t *option) const {
        std::cout << "[ EcalVetoResult ]:\n" << "\t Passes veto : " << passesVeto_ << "\n" << std::endl;
    }
}
