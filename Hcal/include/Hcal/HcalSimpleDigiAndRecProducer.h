#ifndef HCALSIMPLEDIGIANDRECPRODUCER_H
#define HCALSIMPLEDIGIANDRECPRODUCER_H
#include "DetDescr/DetectorID.h"
#include "DetDescr/HcalGeometry.h"
#include "DetDescr/HcalID.h"
#include "Framework/Configure/Parameters.h"
#include "Framework/EventDef.h"
#include "Framework/EventProcessor.h"
#include "Framework/RandomNumberSeedService.h"
#include "TRandom3.h"
#include "Tools/NoiseGenerator.h"

namespace hcal {
class HcalSimpleDigiAndRecProducer : public framework::Producer {
 public:
  HcalSimpleDigiAndRecProducer(const std::string& name,
                               framework::Process& process)
      : framework::Producer{name, process} {}
  ~HcalSimpleDigiAndRecProducer() override = default;
  void configure(framework::config::Parameters& ps) override;
  void SetupRandomNumberGeneration();
  void produce(framework::Event& event) override;

 private:
  std::string input_coll_name{};
  std::string input_pass_name{};
  std::string output_coll_name{};
  double mev_per_mip{};
  double pe_per_mip{};
  double attenuation_length{};
  double mean_noise{};
  double position_resolution{};
  std::unique_ptr<TRandom3> random{nullptr};
  std::unique_ptr<ldmx::NoiseGenerator> noiseGenerator{nullptr};
  int readout_threshold{2};
};

}  // namespace hcal

#endif /* HCALSIMPLEDIGIANDRECPRODUCER_H */
