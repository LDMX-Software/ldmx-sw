
#include "EventProc/MyProcessor.h" 

/************/
/*   LDMX   */
/************/
#include "Event/EcalHit.h"

namespace ldmx { 

    MyProcessor::MyProcessor(const std::string &name, Process &process) : 
        Producer(name, process) { 
    }

    MyProcessor::~MyProcessor() { 
    }

    void MyProcessor::configure(const ParameterSet &pSet) { 
    }

    void MyProcessor::produce(Event& event) { 

        // Check if the collection of digitzed ECal hits exist.  If not, 
        // don't bother processing the event. 
        if (!event.exists("ecalDigis")) return; 

        // Get the collection of digitized ECal hits from the event
        auto hits = event.getCollection("ecalDigis"); 

        // Loop over the collection of hits and print the hit details
        for (int iHit{0}; iHit < hits->GetEntriesFast(); ++iHit) { 
            
            // Retrieve the ith hit from the collection
            auto hit = static_cast<EcalHit*>(hits->At(iHit)); 

            // Print the hit
            hit->Print(); 
        }
    }
} // ldmx

DECLARE_PRODUCER_NS(ldmx, MyProcessor) 
