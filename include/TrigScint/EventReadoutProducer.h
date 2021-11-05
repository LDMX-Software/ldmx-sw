/**
 * @file EventReadoutProducer.h
 * @brief Class that builds linearized full event readout
 * @author Lene Kristian Bryngemark, Stanford University 
 */

#ifndef TRIGSCINT_EVENTREADOUTPRODUCER_H
#define TRIGSCINT_EVENTREADOUTPRODUCER_H

// LDMX
#include "Recon/Event/EventConstants.h"
#include "TrigScint/Event/TrigScintQIEDigis.h"
#include "TrigScint/Event/EventReadout.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

/*~~~~~~~~~~~*/
/* TrigScint */
/*~~~~~~~~~~~*/
#include "TrigScint/SimQIE.h"

namespace trigscint {

/**
 * @class EventReadoutProducer
 * @brief Linearizes ADC info to charge, calculates channel 
 * pedestal and noise levels (in charge)
 */

  class EventReadoutProducer : public framework::Producer {
 public:
  EventReadoutProducer(const std::string& name, framework::Process& process);

  ~EventReadoutProducer();

  /**
   * Callback for the processor to configure itself from the given set
   * of parameters.
   *
   * @param parameters ParameterSet for configuration.
   */
  void configure(framework::config::Parameters& parameters) final override;

  void produce(framework::Event& event);

 private:
  /// Class to set the verbosity level.
  // TODO: Make use of the global verbose parameter.
  bool verbose_{false};

  /// Name of the input collection containing the sim hits
  std::string inputCollection_;

  /// Name of the pass that the input collection is on (empty string means take
  /// any pass)
  std::string inputPassName_;

  /// Name of the output collection that will be used to stored the
  /// digitized trigger scintillator hits
  std::string outputCollection_;

};

}  // namespace trigscint

#endif
