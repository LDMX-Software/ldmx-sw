#include "Tools/HgcrocTriggerCalculations.h"

#include <iostream>
#include "Recon/Event/HgcrocTrigDigi.h"

namespace ldmx {

HgcrocTriggerConditions::HgcrocTriggerConditions(
    const conditions::IntegerTableCondition &ict, bool validate)
    : ict_{ict} {
  if (!validate) return;
  if (ict_.getColumnCount() < 5) {
    EXCEPTION_RAISE(
        "ConditionsException",
        "Inconsistent condition for HgcrocTriggerConditions :" + ict.getName());
  }
  std::vector<std::string> expected_colnames;
  expected_colnames.push_back("ADC_PEDESTAL");
  expected_colnames.push_back("ADC_THRESHOLD");
  expected_colnames.push_back("TOT_PEDESTAL");
  expected_colnames.push_back("TOT_THRESHOLD");
  expected_colnames.push_back("TOT_GAIN");

  for (size_t i = 0; i < 5; i++) {
    if (ict_.getColumnNames()[i] != expected_colnames[i]) {
      EXCEPTION_RAISE("ConditionsException",
                      "Expected column '" + expected_colnames[i] + "', got '" +
                          ict_.getColumnNames()[i] + "'");
    }
  }
}

unsigned int HgcrocTriggerCalculations::singleChannelCharge(
    int adc, int tot, int adc_ped, int adc_thresh, int tot_ped, int tot_thresh,
    int tot_gain) {
  unsigned int charge_adc = 0;

  if (adc > (adc_ped + adc_thresh))
    charge_adc = adc - adc_ped;  // otherwise zero

  unsigned int eff_tot{0};
  if (tot > tot_thresh && tot > tot_ped)
    eff_tot = tot - tot_ped;
  else
    eff_tot = tot_thresh - tot_ped;

  unsigned int charge_tot{0};
  charge_tot = eff_tot * tot_gain;

  unsigned int charge_final = (tot != 0) ? (charge_tot) : (charge_adc);

  return charge_final;
}

HgcrocTriggerCalculations::HgcrocTriggerCalculations(
    const conditions::IntegerTableCondition &ict)
    : conditions_{ict, true} {}

void HgcrocTriggerCalculations::addDigi(unsigned int id, unsigned int tid,
                                        int adc, int tot) {
  unsigned int charge = singleChannelCharge(
      adc, tot, conditions_.adcPedestal(id), conditions_.adcThreshold(id),
      conditions_.totPedestal(id), conditions_.totThreshold(id),
      conditions_.totGain(id));
  if (charge > 0) {
    std::map<unsigned int, unsigned int>::iterator ic = linearCharge_.find(tid);
    if (ic == linearCharge_.end())
      linearCharge_[tid] = charge;
    else
      ic->second += charge;
  }
}

void HgcrocTriggerCalculations::compressDigis(int cells_per_trig) {
  int shift;
  if (cells_per_trig == 4)
    shift = 1;
  else if (cells_per_trig == 9)
    shift = 3;
  else {
    EXCEPTION_RAISE("HgcrocTriggerException",
                    "Invalid number of precision cells per trigger cell: " +
                        std::to_string(cells_per_trig));
  }

  for (auto ilinear : linearCharge_) {
    unsigned int lcharge = ilinear.second;
    lcharge = lcharge >> shift;
    uint8_t ccharge = ldmx::HgcrocTrigDigi::linear2Compressed(lcharge);
    compressedCharge_[ilinear.first] = ccharge;
  }
}

}  // namespace ldmx
