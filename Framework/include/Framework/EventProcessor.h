/**
 * @file EventProcessor.h
 * @brief Base classes for all user event processing components to extend
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_EVENTPROCESSOR_H_
#define FRAMEWORK_EVENTPROCESSOR_H_

/*~~~~~~~~~~~*/
/*   Event   */
/*~~~~~~~~~~~*/
#include "Exception/Exception.h"
#include "Event/RunHeader.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Event.h"
#include "Framework/FrameworkDef.h"
#include "Framework/Parameters.h" 
#include "Framework/StorageControl.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <map>

class TDirectory;

namespace ldmx {

    class Process;
    class EventProcessor;
    class EventFile; 

    /** Typedef for EventProcessorFactory use. */
    typedef EventProcessor* EventProcessorMaker(const std::string& name, Process& process);

    /**
     * @class AbortEventException
     *
     * @brief Specific exception used to abort an event.
     */
    class AbortEventException : public Exception {

        public:

            /**
             * Constructor
             */
            AbortEventException() throw ()
                : Exception( "AbortEventException" , "I should have been caught earlier!" , "" , 0 , "" ) { }
    
            /**
             * Destructor
             */
            virtual ~AbortEventException() throw () { }

    };

    /**
     * @class EventProcessor
     * @brief Base class for all event processing components
     */
    class EventProcessor {

        public:

            /**
             * Class constructor.
             * @param name Name for this instance of the class.
             * @param process The Process class associated with EventProcessor, provided by the framework.
             *
             * @note The name provided to this function should not be
             * the class name, but rather a logical label for this instance of
             * the class, as more than one copy of a given class can be loaded
             * into a Process with different parameters.  Names should not include
             * whitespace or special characters.
             */
            EventProcessor(const std::string& name, Process& process);

            /**
             * Class destructor.
             */
            virtual ~EventProcessor() { }

            /**
             * Callback for the EventProcessor to configure itself from the 
             * given set of parameters.
             * 
             * @param parameters ParameterSet for configuration.
             */
            virtual void configure(Parameters& ) { } 

            /**
             * Callback for the EventProcessor to take any necessary
             * action when the run being processed changes.
             * @param The RunHeader containing run information.
             */
            virtual void onNewRun(const RunHeader& ) {
            }

            /**
             * Callback for the EventProcessor to take any necessary
             * action when a new event input ROOT file is opened.
             * @param Input event ROOT file
             * @note This callback is rarely used.
             */
            virtual void onFileOpen(EventFile& ) {
            }

            /**
             * Callback for the EventProcessor to take any necessary
             * action when a event input ROOT file is closed.
             * @param Input event ROOT file
             * @note This callback is rarely used.
             */
            virtual void onFileClose(EventFile& ) {
            }

            /**
             * Callback for the EventProcessor to take any necessary
             * action when the processing of events starts, such as
             * creating histograms.
             */
            virtual void onProcessStart() {
            }

            /**
             * Callback for the EventProcessor to take any necessary
             * action when the processing of events finishes, such as
             * calculating job-summary quantities.
             */
            virtual void onProcessEnd() {
            }

            /** Access/create a directory in the histogram file for this event
             * processor to create histograms and analysis tuples.
             * @note This method makes the returned directory the current directory
             *     so that newly created objects should go into that directory
             */
            TDirectory* getHistoDirectory();


            /** Mark the current event as having the given storage control hint from this module
             * @param controlhint The storage control hint to apply for the given event
             */
            void setStorageHint(ldmx::StorageControlHint hint) {
                setStorageHint(hint,"");
            }

            /** Mark the current event as having the given storage control hint from this module and the given purpose string
             * @param controlhint The storage control hint to apply for the given event
             * @param purposeString A purpose string which can be used in the skim control configuration
             */
            void setStorageHint(ldmx::StorageControlHint hint, const std::string& purposeString);
    
            /**
             * Internal function which is part of the EventProcessorFactory machinery.
             * @param classname The class name of the processor.
             * @param classtype The class type of the processor (1 for Producer, 2 for Analyzer).
             */
            static void declare(const std::string& classname, int classtype, EventProcessorMaker*);
      
        protected:

            /**
             * Abort the event immediately.
             *
             * Skip the rest of the sequence and don't save anything in the event bus.
             */
            void abortEvent() { throw AbortEventException(); }

            /** Handle to the Process. */
            Process& process_;

        private:

            /** The name of the EventProcessor. */
            std::string name_;

            /** Histogram directory */
            TDirectory* histoDir_{0};
    };

    /**
     * @class Producer
     * @brief Base class for a module which produces a data product.
     *
     * @note This class processes a mutable copy of the event so that it can add data to it.
     */
    class Producer : public EventProcessor {

        public:

            /** Constant used to track EventProcessor types by the EventProcessorFactory */
            static const int CLASSTYPE{1};

            /**
             * Class constructor.
             * @param name Name for this instance of the class.
             * @param process The Process class associated with EventProcessor, provided by the framework
             *
             * @note Derived classes must have a constructor of the same interface, which is the
             * only constructor which will be called by the framework
             *
             * @note The provided name should not be the class name, but rather a logical label for
             * this instance of the class, as more than one copy of a given class can be loaded
             * into a Process with different parameters.  Names should not include
             * whitespace or special characters.
             */
            Producer(const std::string& name, Process& process);

            /**
             * Process the event and put new data products into it.
             * @param event The Event to process.
             */
            virtual void produce(Event& event) = 0;
    };

    /**
     * @class Analyzer
     * @brief Base class for a module which does not produce a data product.
     *
     * @note This class processes a constant copy of the event which cannot be updated.
     */
    class Analyzer : public EventProcessor {

        public:

            /** Constant used to track EventProcessor types by the EventProcessorFactory */
            static const int CLASSTYPE{2};

            /**
             * Class constructor.
             *
             * @param name Name for this instance of the class.
             * @param process The Process class associated with EventProcessor, provided by the framework
             *
             * @note Derived classes must have a constructor of the same interface, which is the
             * only constructor which will be called by the framework
             *
             * @note The provided name should not be the class name, but rather a logical label for
             * this instance of the class, as more than one copy of a given class can be loaded
             * into a Process with different parameters.  Names should not include
             * whitespace or special characters.
             */
            Analyzer(const std::string& name, Process& process);

            /**
             * Process the event and make histograms or summaries
             * @param event The Event to analyze
             */
            virtual void analyze(const Event& event) = 0;

    };

}

/**
 * @def DECLARE_PRODUCER(CLASS)
 * @param CLASS The name of the class to register, which must not be in a namespace.  If the class is in a namespace, use DECLARE_PRODUCER_NS()
 * @brief Macro which allows the framework to construct a producer given its name during configuration.
 * @attention Every Producer class must call this macro or DECLARE_PRODUCER_NS() in the associated implementation (.cxx) file.
 */
#define DECLARE_PRODUCER(CLASS) ldmx::EventProcessor*  CLASS ## _ldmx_make (const std::string& name, ldmx::Process& process) { return new CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmx_declare() { ldmx::EventProcessor::declare(#CLASS,::ldmx::Producer::CLASSTYPE,& CLASS ## _ldmx_make ); }

/**
 * @def DECLARE_ANALYZER(CLASS)
 * @param CLASS The name of the class to register, which must not be in a namespace.  If the class is in a namespace, use DECLARE_PRODUCER_NS()
 * @brief Macro which allows the framework to construct an analyzer given its name during configuration.
 * @attention Every Analyzer class must call this macro or DECLARE_ANALYZER_NS() in the associated implementation (.cxx) file.
 */
#define DECLARE_ANALYZER(CLASS)  ldmx::EventProcessor*  CLASS ## _ldmx_make (const std::string& name, ldmx::Process& process) { return new CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmx_declare() { ldmx::EventProcessor::declare(#CLASS,::ldmx::Analyzer::CLASSTYPE,& CLASS ## _ldmx_make); }

/**
 * @def DECLARE_PRODUCER_NS(NS,CLASS)
 * @param NS The full namespace specification for the Producer
 * @param CLASS The name of the class to register
 * @brief Macro which allows the framework to construct a producer given its name during configuration.
 * @attention Every Producer class must call this macro or DECLARE_PRODUCER() in the associated implementation (.cxx) file.
 */
#define DECLARE_PRODUCER_NS(NS, CLASS) ldmx::EventProcessor*  CLASS ## _ldmx_make (const std::string& name, ldmx::Process& process) { return new NS::CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmx_declare() { ldmx::EventProcessor::declare(std::string(#NS)+"::"+std::string(#CLASS),::ldmx::Producer::CLASSTYPE,& CLASS ## _ldmx_make ); }

/**
 * @def DECLARE_ANALYZER_NS(NS,CLASS)
 * @param NS The full namespace specification for the Analyzer
 * @param CLASS The name of the class to register
 * @brief Macro which allows the framework to construct an analyzer given its name during configuration.
 * @attention Every Analyzer class must call this macro or DECLARE_ANALYZER() in the associated implementation (.cxx) file.
 */
#define DECLARE_ANALYZER_NS(NS, CLASS)  ldmx::EventProcessor*  CLASS ## _ldmx_make (const std::string& name, ldmx::Process& process) { return new NS::CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmx_declare() { ldmx::EventProcessor::declare(std::string(#NS)+"::"+std::string(#CLASS),::ldmx::Analyzer::CLASSTYPE,& CLASS ## _ldmx_make); }

#endif
