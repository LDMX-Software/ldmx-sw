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

        HcalDigiProducer(const std::string& name, const Process& process);

        virtual ~HcalDigiProducer() {
            delete hits_;
            if (random_)
                delete random_;
        }

        virtual void configure(const ParameterSet&);

        virtual void produce(Event& event);

    private:

        static const float FIRST_LAYER_ZPOS;
        static const float LAYER_ZWIDTH;
        static const int NUM_HCAL_LAYERS;
        static const float MEV_PER_MIP;
        static const float PE_PER_MIP;

        TClonesArray* hits_{nullptr};
        TRandom* random_{0};
        std::map<layer, zboundaries> hcalLayers_;
        bool verbose_{false};
        DetectorID* detID_{nullptr};

        float meanNoise_{0};
        int nProcessed_{0};
};

}

#endif
