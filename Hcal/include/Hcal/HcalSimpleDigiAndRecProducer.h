#ifndef HCALSIMPLEDIGIANDRECPRODUCER_H
#define HCALSIMPLEDIGIANDRECPRODUCER_H
#include <random>

#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"
#include "Framework/RandomNumberSeedService.h"
#include "Tools/NoiseGenerator.h"

namespace hcal {
class HcalSimpleDigiAndRecProducer : public framework::Producer {
 public:
  HcalSimpleDigiAndRecProducer(const std::string& name,
                               framework::Process& process)
      : framework::Producer{name, process} {}
  ~HcalSimpleDigiAndRecProducer() override = default;
  void onNewRun(const ldmx::RunHeader& runHeader) override;
  void configure(framework::config::Parameters& ps) override;
  void produce(framework::Event& event) override;

 private:
  std::string input_coll_name_{};
  std::string input_pass_name_{};
  std::string output_coll_name_{};
  double mev_per_mip_{};
  double pe_per_mip_{};
  double attenuation_length_{};
  double mean_noise_{};
  std::mt19937 rng_{};
  std::unique_ptr<std::normal_distribution<double>> position_resolution_smear_{
      nullptr};
  std::unique_ptr<ldmx::NoiseGenerator> noiseGenerator_{nullptr};
  int readout_threshold_{2};
};

}  // namespace hcal

#endif /* HCALSIMPLEDIGIANDRECPRODUCER_H */
