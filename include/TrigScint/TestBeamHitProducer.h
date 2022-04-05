/**
 * @file TestBeamHitProducer.h
 * @brief Class that builds recHits
 * @author Andrew Whitbeck, TTU
 */

#ifndef TRIGSCINT_TESTBEAMHITPRODUCER_H
#define TRIGSCINT_TESTBEAMHITPRODUCER_H

// LDMX
#include "DetDescr/TrigScintID.h"
#include "Recon/Event/EventConstants.h"
#include "TrigScint/Event/TrigScintHit.h"
#include "TrigScint/Event/TestBeamHit.h"
#include "TrigScint/Event/EventReadout.h"


/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"


namespace trigscint {

/**
 * @class TestBeamHitProducer
 * @brief Organizes digis into TrigScintHits, based on linearized 
 * full event readout from test beam/test stand

 */
class TestBeamHitProducer : public framework::Producer {
 public:
  TestBeamHitProducer(const std::string& name, framework::Process& process);

  ~TestBeamHitProducer();

  /**
   * Callback for the processor to configure itself from the given set
   * of parameters.
   *
   * @param parameters ParameterSet for configuration.
   */
  void configure(framework::config::Parameters& parameters) final override;

  void produce(framework::Event& event);

 private:
  /// Set the local verbosity level.
  bool verbose_{false};

  /// Name of the input collection containing the event readout samples
  std::string inputCol_;

  /// Name of the pass that the input collection is on (empty string means take
  /// any pass)
  std::string inputPassName_;

  /// Name of the output collection that will be used to stored the
  /// trigger scintillator hits
  std::string outputCollection_;

  /// SiPM gain, per channel (all initialized to 2e6 in default config)
  std::vector<double> gain_;

  /// channel pedestals [fC]
  std::vector<double> peds_;

  /// start sample for pulse integration (not including any fiber offsets)
  int startSample_{10};

  /// Total number of samples used in pulse integration
  int pulseWidth_{5};
  
  /// Total number of samples used in pulse integration for LYSO (long decay, might need wider window)
  int pulseWidthLYSO_{8};
  
  /// Number of instrumented channels in module
  int nInstrumentedChannels_{12};

/// boolean indicating whether we want to apply quality criteria in hit reconstruction
  int doCleanHits_{false};

};

}  // namespace trigscint

#endif
