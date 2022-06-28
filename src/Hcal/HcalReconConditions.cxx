#include "Hcal/HcalReconConditions.h"

#include "Framework/ConditionsObjectProvider.h"
#include "Framework/EventHeader.h"

namespace hcal {

const std::string HcalReconConditions::CONDITIONS_NAME = "HcalReconConditions";

HcalReconConditions::HcalReconConditions(const conditions::DoubleTableCondition& adc_ped, 
    const conditions::DoubleTableCondition& adc_gain,
    const conditions::DoubleTableCondition& tot_calib)
  : framework::ConditionsObject(HcalReconConditions::CONDITIONS_NAME),
    adc_pedestals_{adc_ped}, adc_gains_{adc_gain},
    tot_calibs_{tot_calib} {}

bool HcalReconConditions::is_adc(const ldmx::HcalDigiID& id, double sum_tot) const {
  // check if the linearization has been done correctly
  //  a non-zero flag value is implicitly converted to true
  if (totCalib(id, i_flagged)) {
    return true;
  }
  
  // if we are in ADC range (which was used as a reference in linearization),
  // we use ADC
  if (sum_tot < totCalib(id, i_lower_offset)) {
    return true;
  }

  return false;
}

double HcalReconConditions::linearize(const ldmx::HcalDigiID& id, double sum_tot) const {
  // we know we have a linearization fit and are in TOT range,
  //  the lower side of TOT needs to be linearized with a specialized power law
  if (sum_tot < totCalib(id, i_cut_point_tot)) {
    return pow(
        (sum_tot - totCalib(id, i_lower_offset)) 
          / totCalib(id, i_low_slope),
        1/totCalib(id,i_low_power)
       ) + totCalib(id, i_tot_not);
  }

  // we know sum_tot is >= lower offset and >= tot cut
  //  higher tot, linearized with adc using a simple linear mapping
  return (sum_tot - totCalib(id, i_high_offset))*totCalib(id, i_high_slope);
}

/**
 * a helpful interface for grabbing the parent conditions at once
 *
 * the most complicated task it does is calculate the "minimum" IOV from 
 * the parent conditions to make sure that it is updated as soon as it needs
 * to be updated.
 *
 * @TODO Right now, no minimum IOV calculation is being done. We just have
 * our IOV be one run at a time.
 */
class HcalReconConditionsProvider : public framework::ConditionsObjectProvider {
  /// name of condition object for hcal adc gains
  std::string adc_gain_;
  /// name of condition object for hcal adc pedestals
  std::string adc_ped_;
  /// name of condition object for hcal tot calibrations
  std::string tot_calib_;
 public:
  /**
   * Retrieve the name of the parent conditions from the configuration
   *
   * @throw Exception if name is not what it should be
   *
   * @param[in] name  should be HcalReconConditions::CONDITIONS_NAME
   * @param[in] tagname
   * @param[in] parameters python configuration parameters
   * @param[in] proc handle to current process
   */
  HcalReconConditionsProvider(const std::string& name, const std::string& tagname,
                              const framework::config::Parameters& parameters,
                              framework::Process& proc)
    : ConditionsObjectProvider(hcal::HcalReconConditions::CONDITIONS_NAME,
                               tagname, parameters, proc) {
      if (name != HcalReconConditions::CONDITIONS_NAME) {
        EXCEPTION_RAISE("BadConfig",
            "The name provided to HcalReconConditionsProvider "+name
            +" is not equal to the expected name "+HcalReconConditions::CONDITIONS_NAME);
      }
      adc_gain_ = parameters.getParameter<std::string>("adc_gain");
      adc_ped_ = parameters.getParameter<std::string>("adc_ped");
      tot_calib_ = parameters.getParameter<std::string>("tot_calib");
    }

  /**
   * Get the wrapped condition
   *
   * This is where we deduce the "minimum" IOV.
   * @note Right now, we just have the IOV go one run at a time.
   *
   * @note Expects the parent condition tables to all be conditions::DoubleTableCondition
   *
   * @see requestParentCondition for how we get the parent condition tables
   */
  virtual std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context) final override {
    // requestParentCondition does check current context for validity 
    // to avoid extra constructions
    auto [ adc_gain_co, adc_gain_iov ] = requestParentCondition(adc_gain_, context);
    auto [ adc_ped_co , adc_ped_iov  ] = requestParentCondition(adc_ped_ , context);
    auto [ tot_calib_co, tot_calib_iov ] = requestParentCondition(tot_calib_, context);
    
    // deduce "minimum" IOV
    //  Framework #56 : https://github.com/LDMX-Software/Framework/issues/56
    auto min_iov = adc_ped_iov;
    // use std::move(min_iov) in return statement below
    
    // wrap
    framework::ConditionsObject* co = new hcal::HcalReconConditions(
        dynamic_cast<const conditions::DoubleTableCondition&>(*adc_ped_co),
        dynamic_cast<const conditions::DoubleTableCondition&>(*adc_gain_co),
        dynamic_cast<const conditions::DoubleTableCondition&>(*tot_calib_co)
        );

    return { co , framework::ConditionsIOV(context.getRun(),context.getRun()) };
  }
};

}  // namespace hcal

DECLARE_CONDITIONS_PROVIDER_NS(hcal, HcalReconConditionsProvider);
