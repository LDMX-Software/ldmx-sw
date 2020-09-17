#ifndef DQM_ECALDIGIVERIFIER_H
#define DQM_ECALDIGIVERIFIER_H

//LDMX Framework
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/Configure/Parameters.h" // Needed to import parameters from configuration file

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

        private:

            /// Collection Name for SimHits
            std::string ecalSimHitColl_;

            /// Pass Name for SimHits
            std::string ecalSimHitPass_;

            /// Collection Name for RecHits
            std::string ecalRecHitColl_;

            /// Pass Name for RecHits
            std::string ecalRecHitPass_;

    };
}

#endif /* DQM_ECALDIGIVERIFIER_H */
