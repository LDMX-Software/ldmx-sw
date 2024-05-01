/**
 * @file TrigScintDigiProducer.h
 * @brief Class that performs digitization of simulated trigger sctintillator
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
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Tools/NoiseGenerator.h"
#include "TrigScint/Event/TrigScintHit.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace trigscint {

enum TrigScintSection {
  UPSTREAM_TAGGER = 1,
  UPSTREAM_TARGET,
  DOWNSTREAM_TARGET,
  NUM_SECTIONS
};

/**
 * @class TrigScintDigiProducer
 * @brief Performs digitization of simulated Trigger Scintillator data
 */
class TrigScintDigiProducer : public framework::Producer {
 public:
  typedef int layer;

  typedef std::pair<double, double> zboundaries;

  TrigScintDigiProducer(const std::string& name, framework::Process& process);

  ~TrigScintDigiProducer();

  /**
   * Callback for the processor to configure itself from the given set
   * of parameters.
   *
   * @param parameters ParameterSet for configuration.
   */
  void configure(framework::config::Parameters& parameters) final override;

  void produce(framework::Event& event);

  ldmx::TrigScintID generateRandomID(int module);

 private:
  /// Random number generator
  std::unique_ptr<TRandom3> random_{nullptr};

  /// Generate noise hits given the number of channels and mean noise.
  std::unique_ptr<ldmx::NoiseGenerator> noiseGenerator_{nullptr};

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

  /// Number of strips per array
  int stripsPerArray_{50};

  /// Number of arrays
  int numberOfArrays_{3};

  /// Mean readout noise
  double meanNoise_{0};

  /// Total MeV per MIP
  double mevPerMip_{1.40};

  /// Total number of photoelectrons per MIP
  double pePerMip_{13.5};
};

}  // namespace trigscint

#endif
