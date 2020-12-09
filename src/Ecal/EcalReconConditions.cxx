#include "Ecal/EcalReconConditions.h"

namespace ldmx {

const std::string EcalReconConditions::CONDITIONS_NAME = "EcalReconConditions";

/**
 * The order of these column names needs to
 * match the indices listed in the header.
 */
const std::vector<std::string> EcalReconConditions::EXPECTED_COLUMNS = {
    "ADC_PEDESTAL",
    "ADC_GAIN",
    "TOT_PEDESTAL",
    "TOT_GAIN"
};

EcalReconConditions::EcalReconConditions(const DoubleTableCondition& table, bool validate)
    : the_table_{table} {

  //leave early if we don't want to validate
  if (!validate) return;

  if (the_table_.getColumnCount() != EXPECTED_COLUMNS.size()) {
    EXCEPTION_RAISE("ConditionsException",
                    "Inconsistent condition for EcalReconConditions :" +
                     table.getName());
  }

  //not using fancy C++17 to shorten this because we want the error to
  // tell us where the failure is
  for (size_t i = 0; i < EXPECTED_COLUMNS.size(); i++) {
    if (the_table_.getColumnNames().at(i) != EXPECTED_COLUMNS.at(i)) {
      EXCEPTION_RAISE("ConditionsException",
                      "Expected column '" + EXPECTED_COLUMNS.at(i) + "', got '" +
                          the_table_.getColumnNames().at(i) + "'");
    }//does name match?
  }//loop through columns
}

} // namespace ldmx
