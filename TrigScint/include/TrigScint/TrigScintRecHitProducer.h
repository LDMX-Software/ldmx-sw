/**
 * @file TrigScintRecHitProducer.h
 * @brief Class that builds recHits
 * @author Andrew Whitbeck, TTU
 */

#ifndef TRIGSCINT_TRIGSCINTDIGIPRODUCER_H
#define TRIGSCINT_TRIGSCINTDIGIPRODUCER_H

#include <string>
#include <vector>

#include "TRandom3.h"

#include "DetDescr/TrigScintID.h"
#include "Recon/Event/EventConstants.h"
#include "Tools/NoiseGenerator.h"
#include "TrigScint/Event/TrigScintHit.h"
#include "TrigScint/Event/TrigScintQIEDigis.h"

#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

#include "TrigScint/SimQIE.h"

namespace trigscint {

class TrigScintRecHitProducer : public framework::Producer {
 public:
  TrigScintRecHitProducer(const std::string& name, framework::Process& process);
  ~TrigScintRecHitProducer();

  void configure(framework::config::Parameters& parameters) override;
  void produce(framework::Event& event) override;

 private:
  void readCalib(const std::string& filename,
                 std::vector<double>* gains,
                 std::vector<double>* pedestals);

  std::string input_collection_;
  std::string input_pass_name_;
  std::string output_collection_;

  double gain_{1e6};
  double pedestal_{6.0};
  double mev_per_mip_{1.40};
  /// Total number of photoelectrons per MIP
  double pe_per_mip_{13.5};
  /// Total number of photoelectrons per MIP
  int sample_of_interest_{2};

  bool use_calib_file_{false};
  std::string calib_file_{""};

  // If > 0: integrate a fixed window starting at sample_of_interest_
  // If <= 0: integrate all samples
  int integration_window_{0};

  std::vector<double> gains_;
  std::vector<double> pedestals_;
};

}  // namespace trigscint

#endif