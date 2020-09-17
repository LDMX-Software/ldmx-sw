/**
 * @file HcalDigiProducer.h
 * @brief Class that performs digitization of simulated HCal data
 * @author Andrew Whitbeck, FNAL
 */

#ifndef EVENTPROC_HCALDIGIPRODUCER_H_
#define EVENTPROC_HCALDIGIPRODUCER_H_

// C++/STL
#include <time.h>

// ROOT
#include "TString.h"
#include "TRandom3.h"

// LDMX
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalID.h"
#include "Event/HcalHit.h"
#include "Framework/EventProcessor.h"
#include "Tools/NoiseGenerator.h"
#include "Framework/Configure/Parameters.h" 

namespace ldmx {

    /**
     * @class HcalDigiProducer
     * @brief Performs digitization of simulated HCal data
     */
    class HcalDigiProducer : public Producer {

        public:
            
            HcalDigiProducer(const std::string& name, Process& process);

            virtual ~HcalDigiProducer() {;}

            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param parameters Set of parameters used to configure this processor.
             */
            void configure(Parameters& parameters) final override;
            
            virtual void produce(Event& event);

      HcalID generateRandomID(HcalID::HcalSection sec);
      void         constructNoiseHit(std::vector<HcalHit>&, HcalID::HcalSection, double, double, const std::map<unsigned int, float>&,std::unordered_set<unsigned int>&);

        private:

            bool                            verbose_{false};
            std::unique_ptr<TRandom3>       random_;
            std::unique_ptr<NoiseGenerator> noiseGenerator_;
            
            double meanNoise_{0};
            int    nProcessed_{0};
            double mev_per_mip_{1.40};
            double pe_per_mip_{13.5};
            double strip_attenuation_length_{100.};
            double strip_position_resolution_{150.};
            std::string sim_hit_pass_name_;
            int    readoutThreshold_{2};
            int    STRIPS_BACK_PER_LAYER_{60};    
            int    NUM_BACK_HCAL_LAYERS_{150};
            int    STRIPS_SIDE_TB_PER_LAYER_{6};
            int    NUM_SIDE_TB_HCAL_LAYERS_{31};
            int    STRIPS_SIDE_LR_PER_LAYER_{31};
            int    NUM_SIDE_LR_HCAL_LAYERS_{63};
            int    SUPER_STRIP_SIZE_{1};
    };

}

#endif
