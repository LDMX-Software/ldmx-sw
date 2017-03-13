/**
 * @file EcalVetoProcessor.h
 * @brief Class that determines if event is vetoable using ECAL hit information
 * @author Owen Colegrove, UCSB
 */

#ifndef EVENTPROC_ECALVETOPROCESSOR_H_
#define EVENTPROC_ECALVETOPROCESSOR_H_

// ROOT
#include "TString.h"
#include "TFile.h"
#include "TTree.h"

// LDMX
#include "DetDescr/EcalDetectorID.h"
#include "Event/EcalVetoResult.h"
#include "Event/Event.h"
#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"

//C++
#include <map>

namespace ldmx {

    class EcalHit;
    class EcalHexReadout;

    /**
     * @class BDTHelper
     * @brief Runs the Boost Decision Tree (BDT) on EcalVetoResult objects using TPython
     */
    class BDTHelper {

        public:

            BDTHelper(TString importBDTFile);

            virtual ~BDTHelper() {}

            void buildFeatureVector(std::vector<float>& bdtFeatures,
                    ldmx::EcalVetoResult& result);

            float getSinglePred(std::vector<float> bdtFeatures);

        private:

            TString vectorToPredCMD(std::vector<float> bdtFeatures);
    };

    /**
     * @class EcalVetoProcessor
     * @brief Determines if event is vetoable using ECAL hit information
     */
    class EcalVetoProcessor: public Producer {

        public:

            typedef std::pair<int, int> LayerCellPair;

            typedef std::pair<int, float> CellEnergyPair;

            typedef std::pair<float, float> XYCoords;

            EcalVetoProcessor(const std::string& name, Process& process) :
                    Producer(name, process) {
            }

            virtual ~EcalVetoProcessor() {
                delete BDTHelper_;
            }

            void configure(const ParameterSet&);

            void produce(Event& event);

        private:

            LayerCellPair hitToPair(EcalHit* hit);

            /* Function to calculate the energy weighted shower centroid */
            int GetShowerCentroidIDAndRMS(const TClonesArray* ecalDigis, double & showerRMS);

            /* Function to load up empty vector of hit maps */
            void fillHitMap(const TClonesArray* ecalDigis,
                    std::vector<std::map<int, float>>& cellMap_);

            /* Function to take loaded hit maps and find isolated hits in them */
            void fillIsolatedHitMap(const TClonesArray* ecalDigis,
                    float globalCentroid,
                    std::vector<std::map<int, float>>& cellMap_,
                    std::vector<std::map<int, float>>& cellMapIso_,
                    bool doTight = false);

            void fillMipTracks(std::vector<std::map<int, float>>& cellMapIso_,
                    std::vector<std::pair<int, float>>& trackVector, int minTrackLen = 2);

        private:

            std::vector<std::map<int, float>> cellMap_;
            std::vector<std::map<int, float>> cellMapLooseIso_;
            std::vector<std::map<int, float>> cellMapTightIso_;

            std::vector<float> ecalLayerEdepRaw_;
            std::vector<float> ecalLayerEdepReadout_;
            std::vector<float> ecalLayerOuterRaw_;
            std::vector<float> ecalLayerOuterReadout_;
            std::vector<float> ecalLayerTime_;

            std::vector<std::pair<int, float>> looseMipTracks_;
            std::vector<std::pair<int, float>> mediumMipTracks_;
            std::vector<std::pair<int, float>> tightMipTracks_;

            int nEcalLayers_{0};
            int backEcalStartingLayer_{0};
            int nReadoutHits_{0};
            int nLooseIsoHits_{0};
            int nTightIsoHits_{0};
            int doBdt_{0};

            double summedDet_{0};
            double summedOuter_{0};
            double backSummedDet_{0};
            double summedLooseIso_{0};
            double maxLooseIsoDep_{0};
            double summedTightIso_{0};
            double maxTightIsoDep_{0};
            double maxCellDep_{0};
            double showerRMS_{0};
            double bdtCutVal_{0};

            EcalVetoResult result_;
            EcalDetectorID detID_;
            bool verbose_{false};
            bool doesPassVeto_{false};

            EcalHexReadout* hexReadout_{nullptr};

            std::string bdtFileName_;
            BDTHelper* BDTHelper_{nullptr};
            std::vector<float> bdtFeatures_;
    };

}

#endif
