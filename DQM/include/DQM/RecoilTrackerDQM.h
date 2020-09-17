/**
 * @file RecoilTrackerDQM.h
 * @brief Analyzer used for DQM of the Recoil tracker. 
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#ifndef _DQM_RECOIL_TRACKER_DQM_H_
#define _DQM_RECOIL_TRACKER_DQM_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <unordered_map>
#include <utility>

//----------//
//   LDMX   //
//----------//
#include "Tools/AnalysisUtils.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "Framework/Configure/Parameters.h" 

namespace ldmx { 

    class RecoilTrackerDQM : public Analyzer { 

        public: 

            /** Constructor */
            RecoilTrackerDQM(const std::string &name, Process &process);

            /** Destructor */
            ~RecoilTrackerDQM(); 
            
            /** 
             * Configure the processor using the given user specified parameters.
             * 
             * @param parameters Set of parameters used to configure this processor.
             */
            void configure(Parameters& parameters) final override; 

            /**
             * Process the event and make histograms ro summaries.
             *
             * @param event The event to analyze.
             */
            void analyze(const Event& event);

            /** Method executed before processing of events begins. */
            void onProcessStart();

    }; // RecoilTrackerDQM 
    
} // ldmx

#endif // _DQM_RECOIL_TRACKER_DQM_H_
