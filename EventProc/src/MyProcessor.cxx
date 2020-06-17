
#include "EventProc/MyProcessor.h" 

namespace ldmx { 

    MyProcessor::MyProcessor(const std::string &name, Process &process) : 
        Producer(name, process) { 
    }

    MyProcessor::~MyProcessor() { 
    }

    void MyProcessor::configure(Parameters& parameters) { 

        /**
         * You access configuration parameters set in the python
         * by asking for the parameter with the same name as the
         * python member variable.
         */

        int my_parameter = parameters.getParameter<int>("my_parameter");

        std::cout << "MyProcessor has my_parameter = " 
            << my_parameter << std::endl;
    }

    void MyProcessor::produce(Event& event) { 

        // Check if the collection of reconstructed ECal hits exist.  If not, 
        // don't bother processing the event. 
        if (!event.exists("EcalRecHits")) return; 

        // Get the collection of digitized ECal hits from the event
        const std::vector<EcalHit> hits = event.getCollection<EcalHit>("EcalRecHits"); 

        // Loop over the collection of hits and print the hit details
        for (const EcalHit &hit : hits ) {
            
            // Print the hit
            hit.Print(); 
        }
    }
} // ldmx

DECLARE_PRODUCER_NS(ldmx, MyProcessor) 
