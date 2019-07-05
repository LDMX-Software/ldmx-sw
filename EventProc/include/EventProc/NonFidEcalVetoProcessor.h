/**
 * @file NonFidEcalVetoProcessor.h
 * @brief Class that determines if events outside fiducial region are vetoable
 * @author Owen Colegrove, UCSB
 */

#ifndef EVENTPROC_NONFIDECALVETOPROCESSOR_H_
#define EVENTPROC_NONFIDECALVETOPROCESSOR_H_

// ROOT
#include "TString.h"
#include "TFile.h"
#include "TTree.h"

// LDMX
#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/EcalDetectorID.h"
#include "Event/NonFidEcalVetoResult.h"
#include "Event/SimTrackerHit.h"
#include "Framework/EventProcessor.h"

//C++
#include <map>

namespace ldmx {

    class EcalHit;
    class EcalHexReadout;

    /**
     * @class NonFidBDTHelper
     * @brief Runs the Boost Decision Tree (BDT) on NonFidEcalVetoResult objects using TPython
     */
    class NonFidBDTHelper {

        public:

            NonFidBDTHelper(TString importBDTFile);

            virtual ~NonFidBDTHelper() {
            }

            void buildFeatureVector(std::vector<float>& bdtFeatures,
                    ldmx::NonFidEcalVetoResult& result);

            float getSinglePred(std::vector<float> bdtFeatures, TString model_name);

        private:

            TString vectorToPredCMD(std::vector<float> bdtFeatures, TString model_name);
    };

    /**
     * @class NonFidEcalVetoProcessor
     * @brief Determines if event is vetoable using ECAL hit information
     */
    class NonFidEcalVetoProcessor: public Producer {

        public:

            typedef std::pair<int, int> LayerCellPair;

            typedef std::pair<int, float> CellEnergyPair;

            typedef std::pair<float, float> XYCoords;

            NonFidEcalVetoProcessor(const std::string& name, Process& process) :
                    Producer(name, process) {
            }

            virtual ~NonFidEcalVetoProcessor() { }

            void configure(const ParameterSet&);

            void produce(Event& event);

        private:

            /** Wrappers for ecalHexReadout functions. See hitToPair().
             *  Necessary to easily combine cellID with moduleID to get unique ID of
             *  hit in layer. In future: combine celID+moduleID+layerID.
             */
            bool isInShowerInnerRing(int centroidID, int probeID){
                return hexReadout_->isNN(centroidID, probeID);
            }
            bool isInShowerOuterRing(int centroidID, int probeID){
                return hexReadout_->isNNN(centroidID, probeID);
            }
            XYCoords getCellCentroidXYPair(int centroidID){
                return hexReadout_->getCellCenterAbsolute(centroidID);
            }
            std::vector<int> getInnerRingCellIds(int cellModuleID){
                return hexReadout_->getNN(cellModuleID);
            }
            std::vector<int> getOuterRingCellIds(int cellModuleID){
                return hexReadout_->getNNN(cellModuleID);
            }

            void clearProcessor();

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

        private:

            std::vector<std::map<int, float>> cellMap_;
            std::vector<std::map<int, float>> cellMapTightIso_;

            std::vector<float> ecalLayerEdepRaw_;
            std::vector<float> ecalLayerEdepReadout_;
            std::vector<float> ecalLayerTime_;
            std::vector<float> mapsx;
            std::vector<float> mapsy;


            int nEcalLayers_{0};
            int backEcalStartingLayer_{0};
            int nReadoutHits_{0};
            int deepestLayerHit_{0};
            int doBdt_{0};


            double summedDet_{0};
            double summedTightIso_{0};
            double maxCellDep_{0};
            double showerRMS_{0};
            double xStd_{0};
            double yStd_{0};
            double avgLayerHit_{0};
            double stdLayerHit_{0};

            std::vector<double> bdtCutVal_{0};

            NonFidEcalVetoResult result_;
            EcalDetectorID detID_;
            bool verbose_{false};
            bool doesPassVeto_{false};

            std::unique_ptr<EcalHexReadout> hexReadout_;

            std::vector<std::basic_string<char>> nfbdtFileNames_;
            std::vector<int> bdtdrop_;
            std::string cellFileNamexy_;
            std::unique_ptr<NonFidBDTHelper> p001BDTHelper_;
            std::unique_ptr<NonFidBDTHelper> p01BDTHelper_;
            std::unique_ptr<NonFidBDTHelper> p1BDTHelper_;
            std::unique_ptr<NonFidBDTHelper> p0BDTHelper_;
            std::vector<float> bdtFeatures_;
    };

}

#endif
