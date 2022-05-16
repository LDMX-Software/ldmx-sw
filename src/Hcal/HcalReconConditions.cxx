#include "Hcal/HcalReconConditions.h"

namespace hcal {

const std::string HcalReconConditions::CONDITIONS_NAME = "HcalReconConditions";

HcalReconConditions(const conditions::DoubleTableCondition& adc_ped, 
    const conditions::DoubleTableCondition& adc_gain,
    const conditions::DoubleTableCondition& tot_ped,
    const conditions::DoubleTableCondition& tot_gain)
  : framework::ConditionsObject(CONDITIONS_NAME),
    adc_pedestals_{adc_ped}, adc_gains_{adc_gain},
    tot_pedestals_{tot_ped}, tot_gains_{tot_gain} {}
}

class HcalReconConditionsProvider : public framework::ConditionsObjectProvider {
  std::map<std::string,std::unique_ptr<conditions::DoubleTableCondition*>>
    recon_conditions_;
 public:
  HcalReconConditionsProvider(const std::string& name, const std::string& tagname,
                              const framework::condfig::Parmaeters& parameters,
                              framework::Process& proc)
    : ConditionsObjectProvider(HcalReconConditions::CONDITIONS_NAME,
                               tagname, parameters, proc) {
      recon_conditions_[parameters.getParameter<std::string>("adc_gain")] = nullptr;
      recon_conditions_[parameters.getParameter<std::string>("adc_ped")] = nullptr;
      recon_conditions_[parameters.getParameter<std::string>("tot_gain")] = nullptr;
      recon_conditions_[parameters.getParameter<std::string>("tot_ped")] = nullptr;
    }

  /// take no action on release becuase smart pointers
  virtual void releaseConditionsObject(const framework::ConditionsObject* co) {}

  virtual std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context) {
    for (auto& [name, table] : recon_conditions_) {
      if (!table) {
        // need to get name
      }
    }


  }
};

}  // namespace hcal
