/**
 * @file EcalDigiVerifier.h
 * @brief Generate histograms to check digi pipeline performance
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef ECAL_ECALDIGIVERIFIER_H
#define ECAL_ECALDIGIVERIFIER_H

//ROOT
#include "TH1.h"
#include "TH2.h"

//LDMX Framework
#include "Event/EventDef.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/ParameterSet.h" // Needed to import parameters from configuration file

namespace ldmx {
    
    /**
     * @class EcalDigiVerifier
     * @brief Generate histograms to check digi pipeline performance
     */
    class EcalDigiVerifier : public ldmx::Analyzer {

        public:

            /**
             * Constructor
             *
             * Blank Analyzer constructor
             */
            EcalDigiVerifier(const std::string& name, ldmx::Process& process) : ldmx::Analyzer(name, process) {}

            /**
             * Input python configuration parameters
             */
            virtual void configure(const ldmx::ParameterSet& ps);

            /**
             * Fills histograms
             */
            virtual void analyze(const ldmx::Event& event);

            /**
             * Creates histograms
             */
            virtual void onProcessStart(); 

            /**
             * Print summary output to terminal
             */
            virtual void onProcessEnd();

        private:

            /** 
             * SimHit E Dep vs Rec Hit Amplitude
             *
             * A perfect reconstruction would see a one-to-one linear relationship between these two.
             * Integrates to number of Rec Hits
             * Aggregates EDeps from any SimHits in the same cell
             */
            TH2F *h_SimEDep_RecAmplitude_;

            /**
             * Total Rec Energy
             *
             * A perfect reconstruction would see a sharp gaussian around the total energy being fired into the ECal in the sample used.
             * (e.g. 4GeV electrons)
             * Integrates to number of events
             */
            TH1F *h_TotalRecEnergy_;

            /**
             * Number of SimHits per each cell
             *
             * Only including cells that have at least one hit
             * Integrates to number of rec hits
             */
            TH1F *h_NumSimHitsPerCell_;

    };
}

#endif /* ECAL_ECALDIGIVERIFIER_H */
