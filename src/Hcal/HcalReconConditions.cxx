#include "Hcal/HcalReconConditions.h"

#include "Framework/ConditionsObjectProvider.h"

namespace hcal {

const std::string HcalReconConditions::CONDITIONS_NAME = "HcalReconConditions";

HcalReconConditions::HcalReconConditions(const conditions::DoubleTableCondition& adc_ped, 
    const conditions::DoubleTableCondition& adc_gain,
    const conditions::DoubleTableCondition& tot_ped,
    const conditions::DoubleTableCondition& tot_gain)
  : framework::ConditionsObject(HcalReconConditions::CONDITIONS_NAME),
    adc_pedestals_{adc_ped}, adc_gains_{adc_gain},
    tot_pedestals_{tot_ped}, tot_gains_{tot_gain} {}

class HcalReconConditionsProvider : public framework::ConditionsObjectProvider {
  std::string adc_gain_, adc_ped_, tot_gain_, tot_ped_;
 public:
  HcalReconConditionsProvider(const std::string& name, const std::string& tagname,
                              const framework::config::Parameters& parameters,
                              framework::Process& proc)
    : ConditionsObjectProvider(hcal::HcalReconConditions::CONDITIONS_NAME,
                               tagname, parameters, proc) {
      adc_gain_ = parameters.getParameter<std::string>("adc_gain");
      adc_ped_ = parameters.getParameter<std::string>("adc_ped");
      tot_gain_ = parameters.getParameter<std::string>("tot_gain");
      tot_ped_ = parameters.getParameter<std::string>("tot_ped");
    }

  virtual std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context) final override {
    auto [ adc_gain_co, adc_gain_iov ] = requestParentCondition(adc_gain_, context);
    auto [ adc_ped_co , adc_ped_iov  ] = requestParentCondition(adc_ped_ , context);
    auto [ tot_gain_co, tot_gain_iov ] = requestParentCondition(tot_gain_, context);
    auto [ tot_ped_co , tot_ped_iov  ] = requestParentCondition(tot_ped_ , context);
    
    // deduce "minimum" IOV
    auto min_iov = adc_ped_iov;
    
    // wrap
    framework::ConditionsObject* co = new hcal::HcalReconConditions(
        dynamic_cast<const conditions::DoubleTableCondition&>(*adc_ped_co),
        dynamic_cast<const conditions::DoubleTableCondition&>(*adc_gain_co),
        dynamic_cast<const conditions::DoubleTableCondition&>(*tot_ped_co),
        dynamic_cast<const conditions::DoubleTableCondition&>(*tot_gain_co)
        );

    return { co , std::move(min_iov) };
  }
};

}  // namespace hcal

DECLARE_CONDITIONS_PROVIDER_NS(hcal, HcalReconConditionsProvider);
