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
    class Process;
    class Event;

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
            int classifyEvent(const int& neutronCount, 
                    const int& protonCount, const int& pionCount, 
                    const int& pi0Count);

            /** Container for 1 dimensional ROOT histograms. */
            std::map<std::string, TH1*> h; 

    };    
    
} // ldmx

#endif // _DQM_ECAL_PN_H_
