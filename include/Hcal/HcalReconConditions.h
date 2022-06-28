
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
  /// index of m_adc_i in tot_calib table
  static const unsigned int i_m_adc_i       = 0;
  /// index of cut_point_tot in tot_calib table
  static const unsigned int i_cut_point_tot = 1;
  /// index of high_slope in tot_calib table
  static const unsigned int i_high_slope    = 2;
  /// index of high_offset in tot_calib table
  static const unsigned int i_high_offset   = 3;
  /// index of low_slope in tot_calib table
  static const unsigned int i_low_slope     = 4;
  /// index of low_power in tot_calib table
  static const unsigned int i_low_power     = 5;
  /// index of lower_offset in tot_calib table
  static const unsigned int i_lower_offset  = 6;
  /// index of tot_not in tot_calib table
  static const unsigned int i_tot_not       = 7;
  /// index of channel in tot_calib table
  static const unsigned int i_channel       = 8;
  /// index of flagged in tot_calib table
  static const unsigned int i_flagged       = 9;
  /// the name of the HcalReconConditions table 
  /// (must match python registration name)
  static const std::string CONDITIONS_NAME;

  /**
   * Provide the necessary tables to hold in one conditions object
   *
   * @param[in] adc_ped double table of ADC pedestals
   * @param[in] adc_gain double table of ADC gains
   * @param[in] tot_ped double table of TOT calibrations
   */
  HcalReconConditions(const conditions::DoubleTableCondition& adc_ped, 
      const conditions::DoubleTableCondition& adc_gain,
      const conditions::DoubleTableCondition& tot_calib);
  
  /**
   * get the ADC pedestal
   *
   * @param[in] id HCal Digi ID for specific chip
   * @param[in] index of column in condition file
   * @returns the ADC pedestal for that chip in counts
   */
  double adcPedestal(const ldmx::HcalDigiID& id, int idx=0) const {
    return adc_pedestals_.get(id.raw(), idx);
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
  double adcGain(const ldmx::HcalDigiID& id, int idx=0) const {
    return adc_gains_.get(id.raw(), idx);
  }

  /**
   * check if the input digi is in ADC mode (or not)
   * using the digi id and its evaluated sum tot
   *
   * @param[in] id HcalDigiID for the channel
   * @param[in] sum_tot already evaluated sum of TOT values
   * @return true if digi is in ADC mode
   */
  bool is_adc(const ldmx::HcalDigiID& id, double sum_tot) const;

  /**
   * linearize the input sum_tot for the input channel
   * into unified amplitude units
   *
   * @note We assume the input channel is already known to be
   * in TOT mode
   *
   * @param[in] id HcalDigiID for the channel
   * @param[in] sum_toto already evaluated sum of TOT values
   * @return linearized amplitude
   */
  double linearize(const ldmx::HcalDigiID& id, double sum_tot) const;

  /**
   * get a TOT calibration value
   *
   * The column indices are stored as static members of this class.
   *
   * @param[in] id HCal Digi ID for specific chip
   * @param[in] index of column in condition file
   * @returns the TOT calibration for that i
   */
  double totCalib(const ldmx::HcalDigiID& id, int idx=0) const {
    return tot_calibs_.get(id.raw(), idx);
  }

 private:
  /// reference to the table of conditions storing the adc pedestals
  const conditions::DoubleTableCondition& adc_pedestals_;
  /// reference to the table of conditions storing the adc gains
  const conditions::DoubleTableCondition& adc_gains_;
  /// reference to the table of conditions storing the tot calibrations
  const conditions::DoubleTableCondition& tot_calibs_;
};  // HcalReconConditions

}  // namespace hcal

#endif  // HCAL_HCALRECONCONDITIONS_H_
