/**
 * @file EcalVetoProcessor.h
 * @brief Class that determines if event is vetoable using ECAL hit information
 * @author Owen Colegrove, UCSB
 */

#ifndef EVENTPROC_ECALVETOPROCESSOR_H_
#define EVENTPROC_ECALVETOPROCESSOR_H_

// ROOT
#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom2.h"
#include "TClonesArray.h"

// LDMX
#include "Event/TriggerResult.h"
#include "Event/EcalHit.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"
#include "Framework/ParameterSet.h"

namespace ldmx {

/**
 * @class EcalVetoProcessor
 * @brief Determines if event is vetoable using ECAL hit information
 */
class EcalVetoProcessor : public Producer {

    public:

        typedef std::pair<int, int> LayerCellPair;

        typedef std::pair<int, float> CellEnergyPair;

        EcalVetoProcessor(const std::string& name, const Process& process) :
                Producer(name, process) {
        }

        virtual ~EcalVetoProcessor() {;}

        void configure(const ParameterSet&);

        void produce(Event& event);

    private:

        inline LayerCellPair hitToPair(EcalHit* hit) {
            int detIDraw = hit->getID();
            detID_.setRawValue(detIDraw);
            detID_.unpack();
            int layer = detID_.getFieldValue("layer");
            int cellid = detID_.getFieldValue("cell");
            return (std::make_pair(layer, cellid));
        }

    private:

        int nEcalLayers_;
        int nLayersMedCal_; 
        int backEcalStartingLayer_;
        double totalDepCut_;
        double totalIsoCut_;
        double backEcalCut_;
        double ratioCut_;

        TriggerResult result_;
        EcalDetectorID detID_;
        bool verbose_{false};
        bool doesPassVeto_{false};
        EcalHexReadout* hexReadout_{nullptr};
};

}

#endif
