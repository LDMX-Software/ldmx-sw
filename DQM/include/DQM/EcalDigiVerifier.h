/**
 * @file EcalDigiVerifier.h
 * @brief Generate histograms to check digi pipeline performance
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef DQM_ECALDIGIVERIFIER_H
#define DQM_ECALDIGIVERIFIER_H

//STL
#include <map>

//ROOT
#include "TH1.h"
#include "TH2.h"

//LDMX Framework
#include "Event/EventDef.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/Parameters.h" // Needed to import parameters from configuration file

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
            virtual void configure(Parameters& ps);

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

            /** Name of collection and passes for sim and rec hits */
            std::string ecalSimHitColl_;
            std::string ecalSimHitPass_;
            std::string ecalRecHitColl_;
            std::string ecalRecHitPass_;

            /** 
             * SimHit E Dep vs Rec Hit Amplitude
             *
             * A perfect reconstruction would see a one-to-one linear relationship between these two.
             * Integrates to number of Rec Hits.
             * Aggregates EDeps from any SimHits in the same cell.
             * The maximum is 25MeV.
             */
            TH2F *h_SimEDep_RecAmplitude_;

            /**
             * Total Rec Energy
             *
             * A perfect reconstruction would see a sharp gaussian around the total energy being fired into the ECal in the sample used.
             * (e.g. 4GeV electrons)
             * Integrates to number of events.
             * The maximum is 8000MeV.
             */
            TH1F *h_TotalRecEnergy_;

            /**
             * Number of SimHits per each cell
             *
             * Only including cells that have at least one hit.
             * Integrates to number of rec hits.
             */
            TH1F *h_NumSimHitsPerCell_;

    };
}

#endif /* DQM_ECALDIGIVERIFIER_H */
