#include "Hcal/HcalReconConditions.h"

namespace hcal {

const std::string HcalReconConditions::CONDITIONS_NAME = "HcalReconConditions";

/**
 * The order of these column names needs to
 * match the indices listed in the header.
 */
const std::vector<std::string> HcalReconConditions::EXPECTED_COLUMNS = {
    "ADC_PEDESTAL", "ADC_GAIN", "TOT_PEDESTAL", "TOT_GAIN"};

HcalReconConditions::HcalReconConditions(
    const conditions::DoubleTableCondition& table, bool validate)
    : the_table_{table} {
  // leave early if we don't want to validate
  if (!validate) return;

  if (the_table_.getColumnCount() != EXPECTED_COLUMNS.size()) {
    EXCEPTION_RAISE(
        "ConditionsException",
        "Inconsistent condition for HcalReconConditions :" + table.getName());
  }

  // not using fancy C++17 to shorten this because we want the error to
  // tell us where the failure is
  for (size_t i = 0; i < EXPECTED_COLUMNS.size(); i++) {
    if (the_table_.getColumnNames().at(i) != EXPECTED_COLUMNS.at(i)) {
      EXCEPTION_RAISE("ConditionsException",
                      "Expected column '" + EXPECTED_COLUMNS.at(i) +
                          "', got '" + the_table_.getColumnNames().at(i) + "'");
    }  // does name match?
  }    // loop through columns
}

}  // namespace hcal
