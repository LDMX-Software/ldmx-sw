/**
 * @file HgcrocTriggerCalculations.h
 * @brief Class that contains the Hgcroc Trigger algorithms, used for both Ecal
 * and Hcal
 * @author Jeremiah Mans, University of Minnesota
 */

#ifndef TOOLS_HGCROCTRIGGERCALCULATIONS_H_
#define TOOLS_HGCROCTRIGGERCALCULATIONS_H_

#include <map>
#include "Conditions/SimpleTableCondition.h"

namespace ldmx {

/**
 * Class to wrap around an integer table of conditions.
 *
 * This hardcodes the column numbers and checks that
 * the hardcoded numbers match the imported columns
 * during construction.
 */
class HgcrocTriggerConditions {
 public:
  /// column index for ADC pedestal
  static const unsigned int IADC_PEDESTAL = 0;
  /// column index for ADC threshold
  static const unsigned int IADC_THRESHOLD = 1;
  /// column index for TOT pedestal
  static const unsigned int ITOT_PEDESTAL = 2;
  /// column index for TOT threshold
  static const unsigned int ITOT_THRESHOLD = 3;
  /// column index for TOT gain
  static const unsigned int ITOT_GAIN = 4;

  /**
   * Constructor
   *
   * Assign the conditions table to use and (if validate is true),
   * check if the hard-coded column indices correctly match
   * the table that is passed.
   *
   * @raises Exception if any of the column names from the passed table
   * do not match the hardcoded indices.
   */
  HgcrocTriggerConditions(const conditions::IntegerTableCondition &,
                          bool validate = true);

  /**
   * get the ADC pedestal
   *
   * @param[in] id raw ID for specific chip
   * @returns the ADC pedestal for that chip
   */
  int adcPedestal(unsigned int id) const { return ict_.get(id, IADC_PEDESTAL); }

  /**
   * get the ADC threshold
   *
   * @param[in] id raw ID for specific chip
   * @returns the ADC threshold for that chip
   */
  int adcThreshold(unsigned int id) const {
    return ict_.get(id, IADC_THRESHOLD);
  }

  /**
   * get the TOT pedestal
   *
   * @param[in] id raw ID for specific chip
   * @returns the TOT pedestal for that chip
   */
  int totPedestal(unsigned int id) const { return ict_.get(id, ITOT_PEDESTAL); }

  /**
   * get the TOT threshold
   *
   * @param[in] id raw ID for specific chip
   * @returns the TOT threshold for that chip
   */
  int totThreshold(unsigned int id) const {
    return ict_.get(id, ITOT_THRESHOLD);
  }

  /**
   * get the TOT gain
   *
   * @param[in] id raw ID for specific chip
   * @returns the TOT gain for that chip
   */
  int totGain(unsigned int id) const { return ict_.get(id, ITOT_GAIN); }

 private:
  /// reference to the table of conditions storing the chip conditions
  const conditions::IntegerTableCondition &ict_;
};  // HgcrocTriggerConditions

/**
 * @class HgcrocTriggerCalculations
 * @brief Contains the core logic for the Hgcroc trigger calculations
 *
 * This class is meant to be created once per event along
 * with the conditions for the chip. These chip conditions are
 * wrapped in an HgcrocTriggerConditions class for easier access.
 * These chip conditions may change from event-to-event,
 * so it is best to allow for this object to change from event-to-event.
 */
class HgcrocTriggerCalculations {
 public:
  /**
   * Calculates the linear trigger charge for a single precision channel (before
   * 2x2 or 3x3 summing)
   * @param adc         ADC measurement for the channel
   * @param tot         TOT measurement for the channel or zero if no TOT
   * @param adc_ped     Trigger pedestal to subtract (8 bits, per channel)
   * @param adc_thresh  ADC threshold for noise-suppression in the trigger path
   * (5 bits, global for ROC)
   * @param tot_ped     TOT pedestal (7 bits, group-of-nine)
   * @param tot_thresh  TOT threshold, use this fixed value as "TOT" if the TOT
   * is below this value (8 bits, group-of-nine)
   * @param tot_gain    TOT to charge multiplicative factor (5 bits, integer
   * math, global for ROC)
   * @returns linear trigger charge from a single precision channel
   */
  static unsigned int singleChannelCharge(int adc, int tot, int adc_ped,
                                          int adc_thresh, int tot_ped,
                                          int tot_thresh, int tot_gain);

  /**
   * Construct the chip trigger calculator
   *
   * Here we pass the table of chip conditions that is used
   * to emulate the chip's behavior for all of the different channel IDs.
   *
   * We also use this opportunity to double check that the column
   * indices in HgcrocTriggerConditions match the column indices
   * imported from the input table.
   *
   * @param[in] ict table of chip conditions
   */
  HgcrocTriggerCalculations(const conditions::IntegerTableCondition &ict);

  /**
   * Determine the linear charge for the given channel, using the calibration
   * information, and add it to the map of trigger channel IDs to linear charge
   * measurements.
   *
   * @see singleChannelCharge for how the precision channel measurement is
   * converted to a linear trig-digi charge
   * @param id Precision channel id (used to lookup in the conditions table)
   * @param tid Trigger channel id
   * @param adc ADC measurement of precision channel if not TOT complete
   * @param tot TOT measurement of precision channel if TOT is complete
   */
  void addDigi(unsigned int id, unsigned int tid, int adc, int tot);

  /**
   * Convert the linear charges to compressed charges, with a division depending
   * on the number of cells summed by HGCROC
   *
   * Fills the map of trigger channel IDs to compressed charge measurements.
   * Some of the lowest order bits are dropped during compression in order
   * to effectively reach the necessary dynamic range. The number of these
   * bits that are dropped depends on the number of cells in each trigger
   * grouping.
   *  - 9 --> drop 8 lowest order bits --> multiply unpacked charge by 8 to get
   * full amount
   *  - 4 --> drop 4 lowest order bits --> mutliply unpacked charge by 4 to get
   * full amount
   *
   * @raises Exception if the input cells_per_trig is not 4 or 9.
   * @param cells_per_trig Valid values are 4 or 9
   */
  void compressDigis(int cells_per_trig);

  /**
   * Access the map of trigger ids to compressed energies
   * @returns const reference to the map of trigger channel ID to compressed
   * charge measurements
   */
  const std::map<unsigned int, uint8_t> &compressedEnergies() const {
    return compressedCharge_;
  }

 private:
  /** The conditions to be used */
  HgcrocTriggerConditions conditions_;
  /** A map of trigger channel id to linear charge */
  std::map<unsigned int, unsigned int> linearCharge_;
  /** A map of trigger channel id to compressed charge */
  std::map<unsigned int, uint8_t> compressedCharge_;
};  // HgcrocTriggerCalculations

}  // namespace ldmx

#endif  // TOOLS_HGCROCTRIGGERCALCULATIONS_H_
