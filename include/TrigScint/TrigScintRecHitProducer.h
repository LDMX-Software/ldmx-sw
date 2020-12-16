/**
 * @file TrigScintRecHitProducer.h
 * @brief Class that builds recHits
 * @author Andrew Whitbeck, TTU
 */

#ifndef EVENTPROC_TRIGSCINTDIGIPRODUCER_H
#define EVENTPROC_TRIGSCINTDIGIPRODUCER_H

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom3.h"

// LDMX
#include "DetDescr/TrigScintID.h"
#include "Recon/Event/EventConstants.h"
#include "TrigScint/Event/TrigScintHit.h"
#include "TrigScint/Event/TrigScintQIEDigis.h"
#include "Tools/NoiseGenerator.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/EventProcessor.h"
#include "Framework/Configure/Parameters.h" 

/*~~~~~~~~~~~*/
/* TrigScint */
/*~~~~~~~~~~~*/
#include "TrigScint/SimQIE.h"

namespace ldmx {

/**
 * @class TrigScintRecHitProducer
 * @brief Organizes digis into TrigScintHits, linearizes TDC 
 * and ADC info, and converts amplitudes to PEs
 
 */
class TrigScintRecHitProducer : public Producer {

 public:

  TrigScintRecHitProducer(const std::string& name, Process& process);

  ~TrigScintRecHitProducer(); 

  /**
   * Callback for the processor to configure itself from the given set
   * of parameters.
   * 
   * @param parameters ParameterSet for configuration.
   */
  void configure(Parameters& parameters) final override;

  void produce(Event& event);

 private:

  /// Class to set the verbosity level.  
  // TODO: Make use of the global verbose parameter. 
  bool verbose_{false};

  /// Name of the input collection containing the sim hits
  std::string inputCollection_;

  /// Name of the pass that the input collection is on (empty string means take any pass)
  std::string inputPassName_;

  /// Name of the output collection that will be used to stored the
  /// digitized trigger scintillator hits
  std::string outputCollection_;

  /// SiPM gain
  double gain_{1e6};

  /// QIE pedestal
  double pedestal_{6.0};

  /// Total MeV per MIP
  double mevPerMip_{1.40};

  /// Total number of photoelectrons per MIP
  double pePerMip_{13.5};

  

};

}

#endif
