#ifndef _DQM_HCAL_DQM_H_
#define _DQM_HCAL_DQM_H_

//----------//
//   STL    //
//----------//
#include <algorithm>

//----------//
//   ROOT   //
//----------//
#include "TVector3.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h" 

#include "Tools/AnalysisUtils.h"
#include "Event/EventDef.h"

namespace ldmx { 

    class HCalDQM : public Analyzer { 
    
        public: 

            /** Constructor */
            HCalDQM(const std::string &name, Process &process);

            /** Destructor */
            ~HCalDQM() { }

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

        private:

            /** The maximum PE threshold used for the veto. */
            float maxPEThreshold_{5}; 
            
    };    
    
} // ldmx

#endif // _DQM_HCAL_DQM_H_
