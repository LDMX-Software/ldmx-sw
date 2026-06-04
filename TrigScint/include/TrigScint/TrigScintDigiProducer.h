/**
 * @file TrigScintDigiProducer.h
 * @brief Class that performs digitization of simulated trigger sctintillator
 * @author Andrew Whitbeck, TTU
 * @author Tamas Almos Vami, UCSB
 */

#ifndef EVENTPROC_TRIGSCINTDIGIPRODUCER_H
#define EVENTPROC_TRIGSCINTDIGIPRODUCER_H

#include <iostream>
#include <random>  //for random num generators

#include "DetDescr/TrigScintID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/Event.h"
#include "Framework/EventProcessor.h"
#include "Framework/Exception/Exception.h"
#include "Framework/RandomNumberSeedService.h"
#include "Recon/Event/EventConstants.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Tools/NoiseGenerator.h"
#include "TrigScint/Event/TrigScintHit.h"

namespace trigscint {

enum TrigScintSection {
  UPSTREAM_TAGGER = 1,
  UPSTREAM_TARGET,
  DOWNSTREAM_TARGET,
  ACTIVE_TARGET,
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

  ~TrigScintDigiProducer() = default;

  /**
   * Callback for the processor to configure itself from the given set
   * of parameters.
   *
   * @param parameters ParameterSet for configuration.
   */
  void configure(framework::config::Parameters& parameters) override;

  void produce(framework::Event& event) override;

  /**
   * Random number generation
   */
  virtual void onNewRun(const ldmx::RunHeader& runHeader) override;

  ldmx::TrigScintID generateRandomID(int module);

 private:
  /// Random number generator
  std::mt19937 rng_;

  /// Generate noise hits given the number of channels and mean noise.
  std::unique_ptr<ldmx::NoiseGenerator> noise_generator_{nullptr};

  /// Name of the input collection containing the sim hits
  std::string input_collection_;

  /// Name of the pass that the input collection is on (empty string means take
  /// any pass)
  std::string input_pass_name_;

  /// Name of the output collection that will be used to stored the
  /// digitized trigger scintillator hits
  std::string output_collection_;

  /// Name of sim particles collection and pass name
  std::string sim_particles_coll_name_;
  std::string sim_particles_passname_;

  /// Number of strips per array
  int strips_per_array_{50};

  /// Number of arrays
  int number_of_arrays_{3};

  /// Mean readout noise
  double mean_noise_{0};

  /// Total MeV per MIP
  double mev_per_mip_{1.40};

  /// Total number of photoelectrons per MIP
  double pe_per_mip_{13.5};
};

}  // namespace trigscint

#endif
