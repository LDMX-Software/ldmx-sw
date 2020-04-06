/**
 * @file DummyProducer.cxx
 * @brief Class that defines a dummy Producer implementation that adds some random data to the event
 * @author Jeremy Mans, University of Minnesota
 */

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <sstream>
#include <vector>

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom.h"

/*~~~~~~~~~~*/
/*   LDMX   */
/*~~~~~~~~~~*/
#include "Framework/Event.h"
#include "Event/EventDef.h"
#include "Framework/EventProcessor.h"
#include "Framework/Parameters.h" 

namespace ldmx {

    /**
     * @class DummyProducer
     * @brief Defines a dummy Producer implementation that adds some random data to the event
     */
    class DummyProducer : public Producer {

        public:

            DummyProducer(const std::string& name, Process& process) :
                Producer(name, process) {
            }

            void configure(Parameters& parameters) final override {
            
                nParticles_ = parameters.getParameter< int >("nParticles");
                aveEnergy_  = parameters.getParameter< double >("aveEnergy"); 
                direction_  = parameters.getParameter< std::vector < double > >("direction"); 
            }

            virtual void produce(Event& event) {
                ldmx_log(debug) << "DummyProducer: Analyzing an event!";

                int iEvent = event.getEventHeader().getEventNumber();
                int np = nParticles_*iEvent;
                std::vector<CalorimeterHit> caloHits;
                std::stringstream ss;
                for (int i = 0; i < np; i++) {
                    caloHits.emplace_back();
                    caloHits.back().setAmplitude( i );
                    caloHits.back().Print(ss);
                    if ( i+1 < np ) ss << "\n";
                }
                ldmx_log(debug) << ss.str();

                event.add("caloHits", caloHits );

            }

            virtual void onFileOpen() {
                ldmx_log(debug) << "DummyProducer: Opening a file!";
            }

            virtual void onFileClose() {
                ldmx_log(debug) << "DummyProducer: Closing a file!";
            }

            virtual void onProcessStart() {
                ldmx_log(debug) << "DummyProducer: Starting processing!";
            }

            virtual void onProcessEnd() {
                ldmx_log(debug) << "DummyProducer: Finishing processing!";
            }

        private:
            int nParticles_{0};
            double aveEnergy_{0};
            std::vector<double> direction_;
            TRandom random_;
    };

}

DECLARE_ANALYZER_NS(ldmx, DummyProducer);
