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

namespace ldmx {

/**
 * @class EcalVetoProcessor
 * @brief Determines if event is vetoable using ECAL hit information
 */
class EcalVetoProcessor : public Producer {

    public:

        typedef std::pair<int, int> layer_cell_pair;

        typedef std::pair<int, float> cell_energy_pair;

        EcalVetoProcessor(const std::string& name, Process& process) :
                Producer(name, process) {
        }

        virtual ~EcalVetoProcessor() {;}

        void configure(const ParameterSet&);

        void produce(Event& event);

    private:

        inline layer_cell_pair hitToPair(EcalHit* hit) {
            int detIDraw = hit->getID();
            detID_.setRawValue(detIDraw);
            detID_.unpack();
            int layer = detID_.getFieldValue("layer");
            int cellid = detID_.getFieldValue("cell");
            return (std::make_pair(layer, cellid));
        }

    private:

        static const int NUM_ECAL_LAYERS;
        static const int NUM_LAYERS_FOR_MED_CAL;
        static const int BACK_ECAL_STARTING_LAYER;
        static const float TOTAL_DEP_CUT;
        static const float TOTAL_ISO_CUT;
        static const float BACK_ECAL_CUT;
        static const float RATIO_CUT;

        TriggerResult result_;
        EcalDetectorID detID_;
        bool verbose_{false};
        bool doesPassVeto_{false};
        EcalHexReadout* hexReadout_{nullptr};
};

}

#endif
