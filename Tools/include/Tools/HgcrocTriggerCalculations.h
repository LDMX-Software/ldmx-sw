/**
 * @file HgcrocTriggerCalculations.h
 * @brief Class that contains the Hgcroc Trigger algorithms, used for both Ecal and Hcal
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef HGCROCTRIGGERCALCULATIONS_H_
#define HGCROCTRIGGERCALCULATIONS_H_

#include <map>
#include "Conditions/SimpleTableCondition.h"

namespace ldmx {

class HgcrocTriggerConditions {
 public:
  static const unsigned int IADC_PEDESTAL  = 0;
  static const unsigned int IADC_THRESHOLD = 1;
  static const unsigned int ITOT_PEDESTAL  = 2;
  static const unsigned int ITOT_THRESHOLD = 3;
  static const unsigned int ITOT_GAIN      = 4;
	
  HgcrocTriggerConditions(const IntegerTableCondition&, bool validate=true);

  /** get the ADC pedestal */
  int adc_pedestal(unsigned int id) const { return ict_.get(id,IADC_PEDESTAL); }

  /** get the ADC pedestal */
  int adc_threshold(unsigned int id) const { return ict_.get(id,IADC_THRESHOLD); }

  /** get the TOT pedestal */
  int tot_pedestal(unsigned int id) const { return ict_.get(id,ITOT_PEDESTAL); }

  /** get the TOT threshold */
  int tot_threshold(unsigned int id) const { return ict_.get(id,ITOT_THRESHOLD); }

  /** get the TOT gain */
  int tot_gain(unsigned int id) const { return ict_.get(id,ITOT_GAIN); }
 private:
  const IntegerTableCondition& ict_;
};

    
/**
 * @class HgcrocTriggerCalculations
 * @brief Contains the core logic for the Hgcroc trigger calculations
 *
 */
class HgcrocTriggerCalculations {
 public:


  /**
   * Calculates the linear trigger charge for a single precision channel (before 2x2 or 3x3 summing)
   * @param adc         ADC measurement for the channel 
   * @param tot         TOT measurement for the channel or zero if no TOT
   * @param adc_ped     Trigger pedestal to subtract (8 bits, per channel)
   * @param adc_thresh  ADC threshold for noise-suppression in the trigger path (5 bits, global for ROC)
   * @param tot_ped     TOT pedestal (7 bits, group-of-nine)
   * @param tot_thresh  TOT threshold, use this fixed value as "TOT" if the TOT is below this value (8 bits, group-of-nine)
   * @param tot_gain    TOT to charge multiplicative factor (5 bits, integer math, global for ROC)
   */
  static unsigned int singleChannelCharge(int adc, int tot,
                                          int adc_ped, int adc_thresh,
                                          int tot_ped, int tot_thresh, int tot_gain);


  HgcrocTriggerCalculations(const IntegerTableCondition& ict);

  /** 
      Determine the linear charge for the given channel, using the calibration information, and add it to the map
      @param id Precision channel id (used to lookup in the conditions table)
      @param tid Trigger channel id
  */
  void addDigi(unsigned int id, unsigned int tid, int adc, int tot);


  /**
     Convert the linear charges to compressed charges, with a division depending on the number of cells summed by HGCROC
     @param cells_per_trig Valid values are 4 or 9
  */
  void compressDigis(int cells_per_trig);

  /** 
   * Access the map of trigger ids to compressed energies
   */
  const std::map<unsigned int, uint8_t>& compressedEnergies() const { return compressedCharge_; }
  
 private:
  /** The conditions to be used */
  HgcrocTriggerConditions conditions_;
  /** A map of trigger channel id to linear charge */
  std::map<unsigned int, unsigned int> linearCharge_;
  /** A map of trigger channel id to compressed charge */
  std::map<unsigned int, uint8_t> compressedCharge_;
};

}

#endif // HGCROCTRIGGERCALCULATIONS_H_
