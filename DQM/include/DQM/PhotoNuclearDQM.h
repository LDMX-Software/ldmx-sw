#ifndef DQM_PHOTONUCLEARDQM_H
#define DQM_PHOTONUCLEARDQM_H

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h" 

class TH1; 

namespace ldmx { 

    // Forward declarations within the ldmx workspace
    class Event;
    class HistogramPool; 
    class Process;
    class SimParticle;

    class PhotoNuclearDQM : public Analyzer { 
    
        public: 

            /// Constructor
            PhotoNuclearDQM(const std::string &name, Process &process);

            /// Destructor
            ~PhotoNuclearDQM();

            /** 
             * Configure this analyzer using the user specified parameters.
             * 
             * @param parameters Set of parameters used to configure this 
             *                   analyzer.
             */
            void configure(Parameters& parameters) final override; 
 
            /**
             * Process the event and create the histogram summaries.
             *
             * @param event The event to analyze.
             */
            void analyze(const Event& event) final override;

            /// Method executed before processing of events begins. 
            void onProcessStart();

        private:

            /** Method used to classify events. */
            int classifyEvent(const SimParticle* particle, const std::map<int,SimParticle> &particleMap, double threshold); 

            /** Method used to classify events in a compact manner. */
            int classifyCompactEvent(const SimParticle* particle, const std::map<int,SimParticle> &particleMap, double threshold); 

            /// Singleton used to access histograms.
            HistogramPool* histograms_{nullptr}; 

    };    
    
} // ldmx

#endif // _DQM_ECAL_PN_H_
