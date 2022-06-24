
#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace hcal {
class HcalSingleEndReconstructor : public framework::Producer {
 public:
  HcalSingleEndReconstructor(const std::string& n, framework::Process& p)
    : Producer(n,p) {}
  virtual void configure(framework::config::Parameters& p) final override;
  virtual void produce(framework::Event& event) final override;
}; // HcalSingleEndReconstructor

void HcalSingleEndReconstructor::configure(framework::config::Parameters& p) {
}

void HcalSingleEndReconstructor::produce(framework::Event& event) {
  const auto& tot_calib_parameters{
    getCondition<conditions::DoubleTableCondition>("hcal_tot_calibration")};
}

}

DECLARE_PRODUCER_NS(hcal,HcalSingleEndReconstructor);
