/**
 * @file EventProcessor.h
 * @brief Base classes for all user event processing components to extend
 * @author Jeremy Mans, University of Minnesota
 */

#ifndef FRAMEWORK_EVENTPROCESSOR_H_
#define FRAMEWORK_EVENTPROCESSOR_H_

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Conditions.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/Exception/Exception.h"
#include "Framework/Factory.h"
#include "Framework/Histograms.h"
#include "Framework/Logger.h"
#include "Framework/NtupleManager.h"
#include "Framework/RunHeader.h"
#include "Framework/StorageControl.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <any>
#include <map>

class t_directory;

namespace framework {

class Process;
class EventProcessor;
class EventFile;

/**
 * @class AbortEventException
 *
 * @brief Specific exception used to abort an event.
 */
class AbortEventException : public framework::exception::Exception {
 public:
  /**
   * Constructor
   *
   * Use empty Exception constructor so stack trace isn't built.
   */
  AbortEventException() throw() : framework::exception::Exception() {}

  /**
   * Destructor
   */
  virtual ~AbortEventException() throw() {}
};

/**
 * @class EventProcessor
 * @brief Base class for all event processing components
 */
class EventProcessor {
 public:
  /// declare that we have a factory for this class
  DECLARE_FACTORY(EventProcessor, EventProcessor *, const std::string &,
                  Process &);

  /**
   * Class constructor.
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor, provided
   * by the framework.
   *
   * @note The name provided to this function should not be
   * the class name, but rather a logical label for this instance of
   * the class, as more than one copy of a given class can be loaded
   * into a Process with different parameters.  Names should not include
   * whitespace or special characters.
   */
  EventProcessor(const std::string &name, Process &process);

  /**
   * Class destructor.
   */
  virtual ~EventProcessor() = default;

  /**
   * Callback for the EventProcessor to configure itself from the
   * given set of parameters.
   *
   * The parameters a processor has access to are the member variables
   * of the python class in the sequence that has className equal to
   * the EventProcessor class name.
   *
   * For an example, look at MyProcessor.
   *
   * @param parameters Parameters for configuration.
   */
  virtual void configure(framework::config::Parameters &parameters) {}

  /**
   * Callback for Producers to add parameters to the run header before
   * conditions are initialized.
   */
  virtual void beforeNewRun(ldmx::RunHeader &run_header) {}

  /**
   * Callback for the EventProcessor to take any necessary
   * action when the run being processed changes.
   * @param run_header The RunHeader containing run information.
   */
  virtual void onNewRun(const ldmx::RunHeader &run_header) {}

  /**
   * Callback for the EventProcessor to take any necessary
   * action when a new event input ROOT file is opened.
   * @param filename Input event ROOT file name.
   * @note This callback is rarely used.
   */
  virtual void onFileOpen(EventFile &event_file) {}

  /**
   * Callback for the EventProcessor to take any necessary
   * action when a event input ROOT file is closed.
   * @param filename Input event ROOT file name
   * @note This callback is rarely used.
   */
  virtual void onFileClose(EventFile &event_file) {}

  /**
   * Callback for the EventProcessor to take any necessary
   * action when the processing of events starts, such as
   * creating histograms.
   */
  virtual void onProcessStart() {}

  /**
   * Callback for the EventProcessor to take any necessary
   * action when the processing of events finishes, such as
   * calculating job-summary quantities.
   */
  virtual void onProcessEnd() {}

  /**
   * How an EventProcessor processes an event.
   * This becomes Producer::produce or Analyzer::analyze
   * depending on which one the user inherits from.
   */
  virtual void process(Event &event) = 0;

  /**
   * Access a conditions object for the current event
   */
  template <class T>
  const T &getCondition(const std::string &condition_name) {
    return getConditions().getCondition<T>(condition_name);
  }

  /**
   * Access/create a directory in the histogram file for this event
   * processor to create histograms and analysis tuples.
   *
   * @note This method makes the returned directory the current directory
   *     so that newly created objects should go into that directory
   *
   * @return TDirectory* reference to directory in histogram file
   */
  TDirectory *getHistoDirectory();

  /** Mark the current event as having the given storage control hint from this
   * module_
   * @param controlhint The storage control hint to apply for the given event
   */
  void setStorageHint(framework::StorageControl::Hint hint) {
    setStorageHint(hint, "");
  }

  /** Mark the current event as having the given storage control hint from this
   * module and the given purpose string
   * @param controlhint The storage control hint to apply for the given event
   * @param purposeString A purpose string which can be used in the skim control
   * configuration
   */
  void setStorageHint(framework::StorageControl::Hint hint,
                      const std::string &purposeString);

  /**
   * Get the current logging frequency from the process
   * @return int frequency logging should occurr
   */
  int getLogFrequency() const;

  /**
   * Get the run number from the process
   * @return int run number
   */
  int getRunNumber() const;

  /**
   * Get the processor name
   */
  std::string getName() const { return name_; }

  /**
   * Internal function which is used to create histograms passed from the python
   * configuration
   * @parma histos vector of Parameters that configure histograms to create
   */
  void createHistograms(
      const std::vector<framework::config::Parameters> &histos);

 protected:
  /**
   * Abort the event immediately.
   *
   * Skip the rest of the sequence and don't save anything in the event bus.
   */
  void abortEvent() { throw AbortEventException(); }

  /// Interface class for making and filling histograms
  HistogramHelper histograms_;

  /// Manager for any ntuples
  NtupleManager &ntuple_{NtupleManager::getInstance()};

  /// The logger for this EventProcessor
  logging::logger the_log_;

 private:
  /**
   * Internal getter for conditions without exposing all of Process
   */
  Conditions &getConditions() const;

  /**
   * Internal getter for EventHeader without exposing all of Process
   */
  const ldmx::EventHeader &getEventHeader() const;

  /** Handle to the Process. */
  Process &process_;

  /** The name of the EventProcessor. */
  std::string name_;

  /** Histogram directory */
  TDirectory *histo_dir_{0};
};

/**
 * @class Producer
 * @brief Base class for a module which produces a data product.
 *
 * @note This class processes a mutable copy of the event so that it can add
 * data to it.
 */
class Producer : public EventProcessor {
 public:
  /**
   * Class constructor.
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor, provided
   * by the framework
   *
   * @note Derived classes must have a constructor of the same interface, which
   * is the only constructor which will be called by the framework
   *
   * @note The provided name should not be the class name, but rather a logical
   * label for this instance of the class, as more than one copy of a given
   * class can be loaded into a Process with different parameters.  Names should
   * not include whitespace or special characters.
   */
  Producer(const std::string &name, Process &process);

  /**
   * Processing an event for a Producer is calling produce
   */
  virtual void process(Event &event) final { produce(event); }

  /**
   * Process the event and put new data products into it.
   * @param event The Event to process.
   */
  virtual void produce(Event &event) = 0;
};

/**
 * @class Analyzer
 * @brief Base class for a module which does not produce a data product.
 *
 * @note This class processes a constant copy of the event which cannot be
 * updated.
 */
class Analyzer : public EventProcessor {
 public:
  /**
   * Class constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor, provided
   * by the framework
   *
   * @note Derived classes must have a constructor of the same interface, which
   * is the only constructor which will be called by the framework
   *
   * @note The provided name should not be the class name, but rather a logical
   * label for this instance of the class, as more than one copy of a given
   * class can be loaded into a Process with different parameters.  Names should
   * not include whitespace or special characters.
   */
  Analyzer(const std::string &name, Process &process);

  /**
   * Processing an event for an Analyzer is calling analyze
   */
  virtual void process(Event &event) final { analyze(event); }

  /**
   * Don't allow Analyzers to add parameters to the run header
   *
   * We make a `final` override of beforeNewRun to be empty so
   * that any derived Analyzer will not be able to compile an
   * override of the beforeNewRun callback.
   */
  virtual void beforeNewRun(ldmx::RunHeader &run_header) final {}

  /**
   * Process the event and make histograms or summaries
   * @param event The Event to analyze
   */
  virtual void analyze(const Event &event) = 0;
};

}  // namespace framework

/**
 * @def DECLARE_PRODUCER(CLASS)
 * @param CLASS The fully-specified name of the class to register
 * (including any namespaces)
 * @brief Macro which allows the framework to construct a producer given its
 * name during configuration.
 * @attention Every Producer class must call this macro
 * in the associated implementation (.cxx) file.
 *
 * We just call #FACTORY_REGISTRATION with framework::EventProcessor
 * as the first argument, so look there for the implementation details.
 */
#define DECLARE_PRODUCER(CLASS) \
  FACTORY_REGISTRATION(framework::EventProcessor, CLASS)

/**
 * @def DECLARE_ANALYZER(CLASS)
 * @param CLASS The fully-specified name of the class to register
 * (including any namespaces)
 * @brief Macro which allows the framework to construct an analyzer given its
 * name during configuration.
 * @attention Every Analyzer class must call this macro
 * in the associated implementation (.cxx) file.
 *
 * We just call #FACTORY_REGISTRATION with framework::EventProcessor
 * as the first argument, so look there for the implementation details.
 */
#define DECLARE_ANALYZER(CLASS) \
  FACTORY_REGISTRATION(framework::EventProcessor, CLASS)

#endif
