#ifndef LDMXSW_FRAMEWORK_EVENTPROCESSOR_H_
#define LDMXSW_FRAMEWORK_EVENTPROCESSOR_H_

#include "Framework/Exception.h"

#include <map>

namespace event {
    class Event;
}

namespace ldmxsw {

    class ParameterSet;
    class Process;
    class EventProcessor;

    /** Typedef for EventProcessorFactory use */
    typedef EventProcessor* EventProcessorMaker(const std::string& name, const Process& process);

    /** Base class for all Event processing 
     */
    class EventProcessor {

	public:

	/** Class constructor 
	 *
	 *@param name Name for this instance of the class.  Should not be
	 * the class name, but rather a logical label for this instance of
	 * the class, as more than one copy of a given class can be loaded
	 * into a Process with different parameters.  Names should not include 
	 * whitespace or special characters. 
	 *@param process The Process class associated with EventProcessor,
	 * provided by the framework
	 */
	EventProcessor(const std::string& name, const Process& process);

	/** 
	 * Callback for the EventProcessor to configure itself from the given set of parameters.
	 * @param parameters ParameterSet for configuration
	 */
        virtual void configure(const ParameterSet& parameters) { }

	/** Callback for the EventProcessor to take any necessary
	 * action when the run being processed changes.
	 * @param run The new run number
	 */
        virtual void onNewRun(int run) { }
	
	/** Callback for the EventProcessor to take any necessary
	 * action when a new event input ROOT file is opened.  
	 * @note This callback is rarely used.
	 * @param filename Input event ROOT file name
	 */
        virtual void onFileOpen(const std::string& filename) { }
	
	/** Callback for the EventProcessor to take any necessary
	 * action when a event input ROOT file is closed.  
	 * @note This callback is rarely used.
	 * @param filename Input event ROOT file name
	 */
        virtual void onFileClose(const std::string& filename) { }
	
	/** Callback for the EventProcessor to take any necessary
	 * action when the processing of events starts, such as
	 * creating histograms.
	 */
	virtual void onProcessStart() { }

	/** Callback for the EventProcessor to take any necessary
	 * action when the processing of events finishes, such as
	 * calculating job-summary quantities
	 */
	virtual void onProcessEnd() { }

	/** Internal function which is part of the EventProcessorFactory machinery */
        static void declare(const std::string& classname, int classtype,EventProcessorMaker*);

	protected:

	/** Handle to the Process */
	const Process& process_;    

	private:
	/** The name of the EventProcessor */
	std::string name_;
    };
    
    /** Base class for a module which produces a data product (thus needs a mutable copy of the event)
     */
    class Producer : public EventProcessor {

	public:

	/** Constant used to track EventProcessor types by the EventProcessorFactory */
        static const int CLASSTYPE{1};

	/** Class constructor 
	 *
	 *@param name Name for this instance of the class.  Should not be
	 * the class name, but rather a logical label for this instance of
	 * the class, as more than one copy of a given class can be loaded
	 * into a Process with different parameters.  Names should not include 
	 * whitespace or special characters. 
	 *@param process The Process class associated with EventProcessor,
	 * provided by the framework
	 *@note Derived classes must have a constructor of the same interface, which is the
	 * only constructor which will be called by the framework
	 */
        Producer(const std::string& name, const Process& process);

	/**
	 * Process the event and put new data products into it
	 * @param event The Event to process
	 */
        virtual void produce(event::Event& event) = 0;    
    };
    
    
    /** Base class for a module which does not produce a data product (thus needs only a constant copy of the event)
     */
    class Analyzer : public EventProcessor {
        
	public:

	/** Constant used to track EventProcessor types by the EventProcessorFactory */
        static const int CLASSTYPE{2};

	/** Class constructor 
	 *
	 *@param name Name for this instance of the class.  Should not be
	 * the class name, but rather a logical label for this instance of
	 * the class, as more than one copy of a given class can be loaded
	 * into a Process with different parameters.  Names should not include 
	 * whitespace or special characters. 
	 *@param process The Process class associated with EventProcessor,
	 * provided by the framework
	 *@note Derived classes must have a constructor of the same interface, which is the
	 * only constructor which will be called by the framework
	 */
	Analyzer(const std::string& name, const Process& process);
    
	/**
	 * Process the event and make histograms or summaries
	 * @param event The Event to analyze
	 */
        virtual void analyze(const event::Event& event) = 0;    

    };

}

/** @def DECLARE_PRODUCER(CLASS)
    @param CLASS The name of the class to register, which must not be in a namespace.  If the class is in a namespace, use DECLARE_PRODUCER_NS()
    @brief Macro which allows the framework to construct a producer given its name during configuration.  
    @attention Every Producer class must call this macro or DECLARE_PRODUCER_NS() in the associated implementation (.cxx) file.
*/
#define DECLARE_PRODUCER(CLASS) ldmxsw::EventProcessor*  CLASS ## _ldmxsw_make (const std::string& name, const ldmxsw::Process& process) { return new CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmxsw_declare() { ldmxsw::EventProcessor::declare(#CLASS,::ldmxsw::Producer::CLASSTYPE,& CLASS ## _ldmxsw_make ); }

/** @def DECLARE_ANALYZER(CLASS)
    @param CLASS The name of the class to register, which must not be in a namespace.  If the class is in a namespace, use DECLARE_PRODUCER_NS()
    @brief Macro which allows the framework to construct an analyzer given its name during configuration.  
    @attention Every Analyzer class must call this macro or DECLARE_ANALYZER_NS() in the associated implementation (.cxx) file.
*/
#define DECLARE_ANALYZER(CLASS)  ldmxsw::EventProcessor*  CLASS ## _ldmxsw_make (const std::string& name, const ldmxsw::Process& process) { return new CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmxsw_declare() { ldmxsw::EventProcessor::declare(#CLASS,::ldmxsw::Analyzer::CLASSTYPE,& CLASS ## _ldmxsw_make); }


/** @def DECLARE_PRODUCER_NS(NS,CLASS)
    @param NS The full namespace specification for the Producer
    @param CLASS The name of the class to register
    @brief Macro which allows the framework to construct a producer given its name during configuration.  
    @attention Every Producer class must call this macro or DECLARE_PRODUCER() in the associated implementation (.cxx) file.
*/
#define DECLARE_PRODUCER_NS(NS,CLASS) ldmxsw::EventProcessor*  CLASS ## _ldmxsw_make (const std::string& name, const ldmxsw::Process& process) { return new NS::CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmxsw_declare() { ldmxsw::EventProcessor::declare(std::string(#NS)+"::"+std::string(#CLASS),::ldmxsw::Producer::CLASSTYPE,& CLASS ## _ldmxsw_make ); }

/** @def DECLARE_ANALYZER_NS(NS,CLASS)
    @param NS The full namespace specification for the Analyzer
    @param CLASS The name of the class to register
    @brief Macro which allows the framework to construct an analyzer given its name during configuration.  
    @attention Every Analyzer class must call this macro or DECLARE_ANALYZER() in the associated implementation (.cxx) file.
*/
#define DECLARE_ANALYZER_NS(NS,CLASS)  ldmxsw::EventProcessor*  CLASS ## _ldmxsw_make (const std::string& name, const ldmxsw::Process& process) { return new NS::CLASS(name,process); }  __attribute__((constructor(1000))) static void CLASS ## _ldmxsw_declare() { ldmxsw::EventProcessor::declare(std::string(#NS)+"::"+std::string(#CLASS),::ldmxsw::Analyzer::CLASSTYPE,& CLASS ## _ldmxsw_make); }



#endif
