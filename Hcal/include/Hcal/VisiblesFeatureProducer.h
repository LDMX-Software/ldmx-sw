#ifndef HCAL_VISIBLESFEATUREPRODUCER_H_
#define HCAL_VISIBLESFEATUREPRODUCER_H_

// LDMX
#include "Framework/Configure/Parameters.h"
#include "Framework/EventProcessor.h"

namespace hcal {

  class VisiblesFeatureProducer : public framework::Analyzer {
  public:
    VisiblesFeatureProducer(const std::string& name, framework::Process& process)
      : Analyzer(name, process) {}

    ~VisiblesFeatureProducer() override = default;

    void configure(framework::config::Parameters& parameters) override;

    void analyze(const framework::Event& event) override;

  private:

    bool training_{false};
    std::string trainingFile_;

    double beamEnergyMeV_{0.};

    std::string hcal_rec_collection_;
    std::string ecal_rec_collection_;
    bool recoil_from_tracking{false};
    std::string track_collection_;
    std::string sp_collection_;

    bool in_list(std::vector<int> parents, int a);

  };
  
} // namespace hcal

#endif
