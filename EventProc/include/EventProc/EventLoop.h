/**
 * @file EventLoop.h
 * @brief Class that implements a basic event processing loop
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef EVENTPROC_EVENTLOOP_H_
#define EVENTPROC_EVENTLOOP_H_

#include "Event/Event.h"
#include "Event/EventFile.h"
#include "Event/EventImpl.h"
#include "EventProc/EventProcessor.h"

using eventproc::EventProcessor;

namespace eventproc {

/**
 * @class EventLoop
 * @brief Basic event processing loop
 *
 * @note
 * This class is only able to process one file in the job.
 * It assumes that the input file contents should be copied
 * to an output file which contains any additional data added
 * by processors during the job.
 */
class EventLoop {

    public:

        /**
         * Class constructor.
         */
        EventLoop() {
        }

        /**
         * Class destructor.
         */
        virtual ~EventLoop() {;}

        /**
         * Activates <i>initialize()</i> methods of registered EventProcessor objects.
         */
        void initialize();

        /**
         * Process a number of events.
         * @param nEvents The number of events to process (-1 for unlimited).
         */
        void run(int nEvents);

        /**
         * Activate <i>finish()</i> methods of registered EventProcessor objects.
         */
        void finish();

        /**
         * Register an EventProcessor.
         * @param eventProcessor The EventProcessor to register.
         */
        void addEventProcessor(EventProcessor* eventProcessor) {
            processors_.push_back(eventProcessor);
        }

        /**
         * Set the default pass name for accessing data from the event collections.
         * @param passName The default pass name.
         */
        void setPassName(std::string passName) {
            passName_ = passName;
        }

        /**
         * Set the name of the output file.
         * @return The name of the output file.
         */
        void setOutputFileName(std::string outputFileName) {
            outputFileName_ = outputFileName;
        }

        /**
         * Set the name of the input file.
         * @return The name of the input file.
         */
        void setInputFileName(std::string inputFileName) {
            inputFileName_ = inputFileName;
        }

    private:

        /**
         * The list of registered EventProcessor objects.
         */
        std::vector<EventProcessor*> processors_;

        /**
         * Wrapper object to EventImpl for accessing event data.
         */
        event::Event* event_{nullptr};

        /**
         * The backing object containing event data.
         */
        event::EventImpl* eventImpl_{nullptr};

        /**
         * Default pass name, which is set to "sim" for reading ldmx-sim data collections.
         */
        std::string passName_{"sim"};

        /**
         * Name of the input file.
         */
        std::string inputFileName_{""};

        /**
         * Name of the output file.
         */
        std::string outputFileName_{""};

        /**
         * EventFile for input.
         */
        event::EventFile* inputFile_{nullptr};

        /**
         * EventFile for output, which will have the collections from the input file
         * plus any collections or objects added by processors during the job.
         */
        event::EventFile* outputFile_{nullptr};
};

}

#endif
