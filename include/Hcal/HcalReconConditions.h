
#ifndef HCAL_HCALRECONCONDTIONS_H_
#define HCAL_HCALRECONCONDTIONS_H_

#include "Conditions/SimpleTableCondition.h"
#include "DetDescr/HcalDigiID.h"

namespace hcal {

/**
 * Class to wrap around the various recon condition tables
 *
 * We expect all of the condition tables to only have two columns
 * (the DetID and the condition itself) so that the column number
 * for getting a value from any of them is zero.
 */
class HcalReconConditions : public framework::ConditionsObject {
 public:
  /// the name of the HcalReconConditions table 
  /// (must match python registration name)
  static const std::string CONDITIONS_NAME;

  /**
   * Provide the necessary tables to hold in one conditions object
   *
   * @param[in] adc_ped double table of ADC pedestals
   * @param[in] adc_gain double table of ADC gains
   * @param[in] tot_ped double table of TOT pedestals
   * @param[in] tot_gain double table of TOT gains
   */
  HcalReconConditions(const conditions::DoubleTableCondition& adc_ped, 
      const conditions::DoubleTableCondition& adc_gain,
      const conditions::DoubleTableCondition& tot_ped,
      const conditions::DoubleTableCondition& tot_gain);

  /**
   * get the ADC pedestal
   *
   * @param[in] id HCal Digi ID for specific chip
   * @returns the ADC pedestal for that chip in counts
   */
  double adcPedestal(const ldmx::HcalDigiID& id) const {
    return adc_pedestals_.get(id.raw(), 0);
  }

  /**
   * get the ADC gain
   *
   * The ADC gain converts the ADC counts measuring
   * a voltage amplitude into an estimated charge
   * deposition [fC].
   *
   * @param[in] id raw ID for specific chip
   * @returns the ADC threshold for that chip in fC/counts
   */
  double adcGain(const ldmx::HcalDigiID& id) const {
    return adc_gains_.get(id.raw(), 0);
  }

  /**
   * get the TOT pedestal
   *
   * @param[in] id HCal Digi ID for specific chip
   * @returns the TOT pedestal for that chip in counts
   */
  double totPedestal(const ldmx::HcalDigiID& id) const {
    return tot_pedestals_.get(id.raw(), 0);
  }

  /**
   * get the TOT gain
   *
   * The TOT gain converts counts measuring time
   * over threshold into an estimate for charge
   * deposited in that cell [fC].
   *
   * @param[in] id HCal Digi ID for specific chip
   * @returns the TOT gain for that chip in fC/counts
   */
  double totGain(const ldmx::HcalDigiID& id) const {
    return tot_gains_.get(id.raw(), 0);
  }

 private:
  /// reference to the table of conditions storing the adc pedestals
  const conditions::DoubleTableCondition& adc_pedestals_;
  /// reference to the table of conditions storing the adc gains
  const conditions::DoubleTableCondition& adc_gains_;
  /// reference to the table of conditions storing the tot pedestals
  const conditions::DoubleTableCondition& tot_pedestals_;
  /// reference to the table of conditions storing the tot gains
  const conditions::DoubleTableCondition& tot_gains_;
};  // HcalReconConditions

}  // namespace hcal

#endif  // HCAL_HCALRECONCONDITIONS_H_
