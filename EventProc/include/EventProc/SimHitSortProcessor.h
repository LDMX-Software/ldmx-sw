/**
 * @file SimHitSortProcessor.h
 * @brief 
 * @author 
 */

#ifndef EVENTPROC_SIMHITPROCESSOR_H_
#define EVENTPROC_SIMHITPROCESSOR_H_

// STL
#include <string>

// LDMX
#include "Event/EventConstants.h"
#include "Framework/EventProcessor.h"

namespace ldmx {

    /**
     * @class SimHitSortProcessor
     * @brief 
     *
     * @note
     * 
     */
    class SimHitSortProcessor : public Producer {

        public:

            /**
             * Class constructor.
             */
            SimHitSortProcessor(const std::string& name, Process& process) :
                Producer(name, process) {
            }


            /**
             * Class destructor.
             */
            virtual ~SimHitSortProcessor() {
                if ( sortedHits ) delete sortedHits;
            }

            /**
             */
            virtual void configure(const ParameterSet& pSet);

            /**
             * description...
             */
            virtual void produce(Event& event);

        private:
            
            TClonesArray *sortedHits;
            std::string collectionName;
            std::string outputCollection;
    };

}

#endif
