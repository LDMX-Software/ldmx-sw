/**
 * @file TrigScintDigiProducer.h
 * @brief Class that performs digitization of simulated trigger sctintillator
 * @author Andrew Whitbeck, TTU
 */

#ifndef EVENTPROC_TRIGSCINTDIGIPRODUCER_H
#define EVENTPROC_TRIGSCINTDIGIPRODUCER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <time.h>

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom3.h"

// LDMX
#include "DetDescr/DefaultDetectorID.h"
#include "Event/EventConstants.h"
#include "Event/TrigScintHit.h"
#include "Event/SimCalorimeterHit.h"
#include "Tools/NoiseGenerator.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h" 

namespace ldmx {

    enum TrigScintSection{
        UPSTREAM_TAGGER,
        UPSTREAM_TARGET,
        DOWNSTREAM_TARGET,
        NUM_SECTIONS
    };

    /**
     * @class TrigScintDigiProducer
     * @brief Performs digitization of simulated Trigger Scintillator data
     */
    class TrigScintDigiProducer : public Producer {

        public:

            typedef int layer;

            typedef std::pair<double, double> zboundaries;

            TrigScintDigiProducer(const std::string& name, Process& process);

            ~TrigScintDigiProducer(); 

            /**
             * Callback for the processor to configure itself from the given set
             * of parameters.
             * 
             * @param parameters ParameterSet for configuration.
             */
            void configure(Parameters& parameters) final override;

            void produce(Event& event);

            unsigned int generateRandomID(TrigScintSection sec);

        private:

            TClonesArray* hits_{nullptr};
            TRandom3* random_{new TRandom3(time(nullptr))};
            bool verbose_{false};
            DefaultDetectorID* detID_{nullptr};
	        std::string input_collection_;
	        std::string output_collection_;
            NoiseGenerator* noiseGenerator_;

            /** Generator for simulating noise hits. */
            double meanNoise_{0};
            int    nProcessed_{0};
            double mev_per_mip_{1.40};
            double pe_per_mip_{13.5};
	        int    NUM_STRIPS_PER_ARRAY_{50};
            int    NUM_ARRAYS_{3};
    };

}

#endif
