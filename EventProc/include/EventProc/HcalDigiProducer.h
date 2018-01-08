/**
 * @file HcalDigiProducer.h
 * @brief Class that performs digitization of simulated HCal data
 * @author Andrew Whitbeck, FNAL
 */

#ifndef EVENTPROC_HCALDIGIPRODUCER_H_
#define EVENTPROC_HCALDIGIPRODUCER_H_

// ROOT
#include "TString.h"
#include "TRandom.h"

// LDMX
#include "DetDescr/DetectorID.h"
#include "Event/SimCalorimeterHit.h"
#include "Framework/EventProcessor.h"

namespace ldmx {

    /**
     * @class HcalDigiProducer
     * @brief Performs digitization of simulated HCal data
     */
    class HcalDigiProducer : public Producer {

        public:

            typedef int layer;

            typedef std::pair<double, double> zboundaries;

            HcalDigiProducer(const std::string& name, Process& process);

            virtual ~HcalDigiProducer() {
                delete hits_;
                if (random_)
                    delete random_;
            }

            virtual void configure(const ParameterSet&);

            virtual void produce(Event& event);


        private:

            TClonesArray* hits_{nullptr};
            TRandom* random_{0};
            std::map<layer, zboundaries> hcalLayers_;
            bool verbose_{false};
            DetectorID* detID_{nullptr};

            double meanNoise_{0};
            int    nProcessed_{0};
            double mev_per_mip_{1.40};
            double pe_per_mip_{13.5};
            int    doStrip_{true};
            std::string simHitCollection_;
    };

}

#endif
