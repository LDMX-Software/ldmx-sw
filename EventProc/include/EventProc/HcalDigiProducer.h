/**
 * @file HcalDigiProducer.h
 * @brief Class that performs digitization of simulated HCal data
 * @author Andrew Whitbeck, FNAL
 */

#ifndef EVENTPROC_HCALDIGIPRODUCER_H_
#define EVENTPROC_HCALDIGIPRODUCER_H_

<<<<<<< HEAD
// C++/STL
#include <time.h>

// ROOT
#include "TString.h"
#include "TRandom3.h"

// LDMX
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalID.h"
#include "Event/EventConstants.h"
#include "Event/HcalHit.h"
#include "Event/SimCalorimeterHit.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"
=======
// ROOT
#include "TString.h"
#include "TRandom.h"

// LDMX
#include "DetDescr/DetectorID.h"
#include "Event/SimCalorimeterHit.h"
#include "Framework/EventProcessor.h"
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

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

<<<<<<< HEAD
            unsigned int generateRandomID(HcalSection sec);
=======
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

        private:

            TClonesArray* hits_{nullptr};
<<<<<<< HEAD
            TRandom3* random_{new TRandom3(time(nullptr))};
            std::map<layer, zboundaries> hcalLayers_;
            bool verbose_{false};
            DetectorID* detID_{nullptr};
            
            /** Generator for simulating noise hits. */
            NoiseGenerator* noiseGenerator_;
=======
            TRandom* random_{0};
            std::map<layer, zboundaries> hcalLayers_;
            bool verbose_{false};
            DetectorID* detID_{nullptr};
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8

            double meanNoise_{0};
            int    nProcessed_{0};
            double mev_per_mip_{1.40};
            double pe_per_mip_{13.5};
<<<<<<< HEAD
            double strip_attenuation_length_{100.};
            double strip_position_resolution_{150.};
            int    readoutThreshold_{2};
            int    doStrip_{true};
            int    STRIPS_BACK_PER_LAYER_{31};
            int    NUM_BACK_HCAL_LAYERS_{150};
            int    STRIPS_SIDE_TB_PER_LAYER_{6};
            int    NUM_SIDE_TB_HCAL_LAYERS_{31};
            int    STRIPS_SIDE_LR_PER_LAYER_{31};
            int    NUM_SIDE_LR_HCAL_LAYERS_{63};
            int    SUPER_STRIP_SIZE_{1};
=======
            int    doStrip_{true};
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
    };

}

#endif
