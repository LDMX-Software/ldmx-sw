/**
 * @file TrigScintQIEDigiProducer.h
 * @brief Class that simulates QIE chip of the trigger scintillator
 * @author Niramay Gogate, Texas Tech University
 */

#ifndef TRIGSCINT_TRIGSCINTQIEDIGIPRODUCER_H
#define TRIGSCINT_TRIGSCINTQIEDIGIPRODUCER_H

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <time.h>

/*~~~~~~~~~~*/
/*   ROOT   */
/*~~~~~~~~~~*/
#include "TRandom3.h"

// LDMX
#include "DetDescr/TrigScintID.h"
#include "SimCore/Event/SimCalorimeterHit.h"
#include "TrigScint/Event/TrigScintHit.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

// QIE output class
#include "TrigScint/Event/TrigScintQIEDigis.h"
#include "TrigScint/SimQIE.h"

namespace trigscint {

enum TrigScintSection {
  UPSTREAM_TAGGER = 1,
  UPSTREAM_TARGET,
  DOWNSTREAM_TARGET,
  NUM_SECTIONS
};
/**
 * @class TrigScintQIEDigiProducer
 * @brief Class that simulates QIE chip of the trigger scintillator
 */
class TrigScintQIEDigiProducer : public framework::Producer {
 public:
  TrigScintQIEDigiProducer(const std::string& name,
                           framework::Process& process);
  virtual ~TrigScintQIEDigiProducer() = default;

  /**
   * Callback for the processor to configure itself from the given set
   * of parameters.
   *
   * @param parameters ParameterSet for configuration.
   */
  void configure(framework::config::Parameters& parameters) final override;

  /**
   * Method to produce a collection of QIE digis.
   * @brief For each event, the SimHit information is converted to an
   * output format analogous to real QIE output.
   */
  void produce(framework::Event& event) override;

 private:
  /// Random number generator
  std::unique_ptr<TRandom3> random_{nullptr};

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

  /// QIE Input pulse shape
  std::string input_pulse_shape_;

  /// QIE Input pulse parameters
  std::vector<float> pulse_params_;

  /// Overall input pulse time offset
  float toff_overall_;

  /// no. of time samples analysed by QIE
  int maxts_;

  /// QIE TDC Current threshold
  float tdc_thr_;

  /// QIE pedestal
  float pedestal_;

  /// QIE electronic noise
  float elec_noise_;

  /// SiPM Gain
  float sipm_gain_;

  /// QIE sampling frequency [in MHz]
  float s_freq_;

  /// Zero-suppression: discard any integrated pulses with PE < this number
  float zeroSuppCut_{1.};

  /// SimQIE pointer
  SimQIE* smq_{nullptr};
};

}  // namespace trigscint

#endif
