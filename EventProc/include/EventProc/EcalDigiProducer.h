/**
 * @file EcalDigiProducer.h
 * @brief Class that performs basic ECal digitization
 * @author Owen Colegrove, UCSB
 */

#ifndef EVENTPROC_ECALDIGIPRODUCER_H_
#define EVENTPROC_ECALDIGIPRODUCER_H_

// ROOT
#include "TString.h"
#include "TRandom.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom2.h"
#include "TClonesArray.h"

// LDMX
#include "Event/SimCalorimeterHit.h"
#include "DetDescr/DetectorID.h"
#include "DetDescr/EcalDetectorID.h"
#include "DetDescr/EcalHexReadout.h"
#include "Framework/EventProcessor.h"

namespace ldmx {

    /**
     * @class EcalDigiProducer
     * @brief Performs basic ECal digitization
     */
    class EcalDigiProducer : public Producer {

        public:

            typedef std::pair<int, int> layer_cell_pair;

            typedef std::pair<int, float> cell_energy_pair;

            EcalDigiProducer(const std::string& name, Process& process);

            virtual ~EcalDigiProducer() {
            }

            virtual void configure(const ParameterSet&);

            virtual void produce(Event& event);

        private:

            inline layer_cell_pair hitToPair(SimCalorimeterHit* hit) {
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

            TRandom2 *noiseInjector_{nullptr};
            TClonesArray* ecalDigis_{nullptr};
            EcalDetectorID detID_;
            EcalHexReadout* hexReadout_{nullptr};
            float meanNoise_{0};
            float readoutThreshold_{0};
            std::string simHitCollection_{0};
    };

}

#endif
