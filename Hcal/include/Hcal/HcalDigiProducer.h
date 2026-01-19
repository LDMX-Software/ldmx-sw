#ifndef HCAL_HCALDIGIPRODUCER_H_
#define HCAL_HCALDIGIPRODUCER_H_

//----------------//
//   C++ StdLib   //
//----------------//
#include <memory>  //for smart pointers
#include <random>  //for random num generators
#include <set>     //for tracking used detector IDs

//----------//
//   LDMX   //
//----------//
#include "DetDescr/HcalDigiID.h"
#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"
#include "Framework/EventProcessor.h"
#include "Framework/RandomNumberSeedService.h"
#include "Recon/Event/EventConstants.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocPulseTruth.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "Tools/HgcrocEmulator.h"
#include "Tools/NoiseGenerator.h"

namespace hcal {

/**
 * @class HcalDigiProducer
 * @brief Performs basic HCal digitization
 */
class HcalDigiProducer : public framework::Producer {
 public:
  /**
   * Constructor
   * Makes unique noise generator and injector for this class
   */
  HcalDigiProducer(const std::string& name, framework::Process& process);

  /// Default destructor
  virtual ~HcalDigiProducer() = default;

  /**
   * Configure this producer from the python configuration.
   * Sets event constants and configures the noise generator, noise injector,
   * and pulse function. Creates digi collection
   */
  void configure(framework::config::Parameters&) override;

  /**
   * Simulates measurement of pulse and creates digi collection for input event.
   */
  void produce(framework::Event& event) override;

  /**
   * Random number generation
   */
  virtual void onNewRun(const ldmx::RunHeader& runHeader) override;

 private:
  ///////////////////////////////////////////////////////////////////////////////////////
  // Python Configuration Parameters

  /// input hit collection name
  std::string input_coll_name_;

  /// input pass name
  std::string input_pass_name_;

  /// output hit collection name
  std::string digi_coll_name_;

  /// output pulse truth collection name
  std::string pulse_truth_coll_name_;

  /// Time interval for chip clock in ns
  double clock_cycle_;

  /// Depth of ADC buffer.
  int n_ad_cs_;

  /// Index for the Sample Of Interest in the list of digi samples
  int i_soi_;

  /// Conversion from energy in MeV to voltage in mV
  double mev_;

  /// Strip attenuation length [m]
  double attlength_;

  ///////////////////////////////////////////////////////////////////////////////////////
  // Other member variables

  /// Put noise into empty channels, not configurable, only helpful in
  /// development
  bool noise_{true};

  /// If true, save the "analog" composite pulse shape in the HGCROC emulator
  /// before it gets digitized
  bool save_pulse_truth_info_{false};

  /// If false, save digis from all channels, even pure noise in empty bars
  /// Helpful when comparing with test beam data
  bool zero_suppression_{true};

  /// Hgcroc Emulator to digitize analog voltage signals
  std::unique_ptr<ldmx::HgcrocEmulator> hgcroc_;

  /// Conversion from time in ns to ticks of the internal clock
  double ns_;

  /// Read out threshold
  double readout_threshold_;
  /// Read out pedestal
  double pedestal_;
  /// Read out gain
  double gain_;
  /// Noise RMS
  double noise_rms_;

  /// Generates noise hits based off of number of cells that are not hit
  std::unique_ptr<ldmx::NoiseGenerator> noise_generator_;

  /// Generates Gaussian noise on top of real hits_
  std::mt19937 rng_;
};
}  // namespace hcal

#endif
