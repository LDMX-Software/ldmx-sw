/**
 * @file EcalPN.h
 * @brief Analyzer used for ECal PN DQM. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _DQM_ECAL_PN_H_
#define _DQM_ECAL_PN_H_

//----------//
//   LDMX   //
//----------//
#include "Framework/EventProcessor.h"

class TH1; 

namespace ldmx { 

    // Forward declarations within the ldmx workspace
    class Event;
    class HistogramPool; 
    class Process;
    class SimParticle;

    class EcalPN : public Analyzer { 
    
        public: 

            /** Constructor */
            EcalPN(const std::string &name, Process &process);

            /** Destructor */
            ~EcalPN(); 
 
            /**
             * Process the event and make histograms ro summaries.
             *
             * @param event The event to analyze.
             */
            void analyze(const Event& event);

            /** Method executed before processing of events begins. */
            void onProcessStart();

        private:

            /** Method used to classify events. */
            int classifyEvent(const SimParticle* particle, double threshold); 

            /** Singleton used to access histograms. */
            HistogramPool* histograms_{nullptr}; 

    };    
    
} // ldmx

#endif // _DQM_ECAL_PN_H_
