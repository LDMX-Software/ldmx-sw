
#ifndef RECON_EXAMPLES_MY_PROCESSOR_H_
#define RECON_EXAMPLES_MY_PROCESSOR_H_

/***************/
/*   ldmx-sw   */
/***************/
#include "Ecal/Event/EcalHit.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace recon {

/**
 * Minimal example of a processor.
 *
 * This processor will loop over all of the ECal hits in an event and
 * print out their details.
 */
class MyProcessor : public framework::Producer {
 public:
  /**
   * Constructor.
   *
   * @param name Name for this instance of the class.
   * @param process The Process class associated with EventProcessor,
   * provided by the framework.
   */
  MyProcessor(const std::string &name, framework::Process &process);

  /// Destructor
  ~MyProcessor();

  /**
   * Configure the processor using the given user specified parameters.
   *
   * The user specified parameters that are availabed are defined
   * in the python configuration class. Look at the my_processor.py
   * module of the EventProc python for the python structure.
   *
   * @param parameters Set of parameters used to configure this processor.
   */
  void configure(framework::config::Parameters &parameters) final override;

  /**
   * Process the event and put new data products into it.
   *
   * @param event The event to process.
   */
  void produce(framework::Event &event);

};  // MyProcessor

}  // namespace recon

#endif  // RECON_EXAMPLES_MY_PROCESSOR_H_
