
#ifndef EVENTPROC_TRIGSCINTCLUSTERPRODUCER_H
#define EVENTPROC_TRIGSCINTCLUSTERPRODUCER_H

//LDMX Framework
#include "Framework/Event.h"
#include "Framework/EventProcessor.h" //Needed to declare processor
#include "Framework/Parameters.h" // Needed to import parameters from configuration file
#include "Event/TrigScintCluster.h"
#include "Event/EventConstants.h"
#include "Event/TrigScintHit.h"


#include "TClonesArray.h"

namespace ldmx {

    /**
     * @class TrigScintClusterProducer
     * @brief 
     */
    class TrigScintClusterProducer : public ldmx::Producer {
        public:

            TrigScintClusterProducer(const std::string& name, ldmx::Process& process) : ldmx::Producer(name, process) {
            }

            virtual void configure(ldmx::Parameters& ps);

            virtual void produce(ldmx::Event& event);

            virtual void addHit( uint idx, TrigScintHit hit ); 

            virtual void onFileOpen();

            virtual void onFileClose();

            virtual void onProcessStart(); 

            virtual void onProcessEnd();

        private:
            std::vector< TrigScintCluster > clusters_;

            double seed_{0.};
            double minThr_{0.};
            int maxWidth_{2};
            int verbose_{0}; 

            std::string input_collection_;
            std::string output_collection_;


            float centroid_{0.};  //channel nb centroid (will be content weighted)
            float val_{0.};       // energy, PE, or sth
            float valE_{0.};       // energy, only; leave val_ for PE
            std::vector <unsigned int> v_addedIndices_;  // book keep which channels have already been added to a cluster
            std::vector <unsigned int> v_usedIndices_;  // book keep which channels have already been added to any cluster
            float beamE_{0.};     // fraction of cluster energy associated with beam electron sim hits
            float time_{0.};

            // empty map container 
            std::map<int, int> hitChannelMap_; 

    };
}

#endif /* EVENTPROC_TRIGSCINTCLUSTERPRODUCER_H */
