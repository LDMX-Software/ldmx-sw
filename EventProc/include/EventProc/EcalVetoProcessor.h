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
#include "DetDescr/EcalHexReadout.h"
#include "DetDescr/EcalDetectorID.h"
#include "Event/EcalVetoResult.h"
#include "Event/SimTrackerHit.h"
#include "Framework/EventProcessor.h"

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

            virtual ~BDTHelper() {
            }

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

            std::vector<XYCoords> getTrajectory(std::vector<double> momentum, std::vector<float> position);

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
            double ecalBackEnergy_{0};
        
            double bdtCutVal_{0};

            EcalVetoResult result_;
            EcalDetectorID detID_;
            bool verbose_{false};
            bool doesPassVeto_{false};

            EcalHexReadout* hexReadout_{nullptr};

            std::string bdtFileName_;
            std::string cellFileNamexy_;
            BDTHelper* BDTHelper_{nullptr};
            std::vector<float> bdtFeatures_;

            /** Name of the collection which will containt the results. */
            std::string collectionName_{"EcalVeto"}; 

    };

}

#endif
