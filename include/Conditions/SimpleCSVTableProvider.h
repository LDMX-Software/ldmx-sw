/**
 * @file ConditionsObjectProvider which can be configured to load conditions
 * objects from CSV files provided via URLs with IOVs defined in the
 * configuration.  Configuration can be either inline or via an external CSV
 * file.
 */
#ifndef CONDITIONS_SIMPLECSVTABLEPROVIDER_H_
#define CONDITIONS_SIMPLECSVTABLEPROVIDER_H_

#include "Framework/ConditionsObjectProvider.h"

namespace conditions {

/**
 * @class ConditionsObjectProvider which can be configured to load conditions
 objects from
 *  CSV files.
 *
 * Configuration via Python uses the following parameters:
 * "dataType", "columns", "entries"
 * "DataType may be "int" or "double".
 * The "columns" parameter is the list of names of data columns to be read out.

 * "entries" is an array of parameter sets.
 * Each entry must contain a "URL"
 * The entry _may_ contain "firstRun", "lastRun", "runType", and "values".
 * If "firstRun" is not provided, -1 is assumed.  If "lastRun" is not provided,
 -1 is assumed.
 * The parameter "runType" could have the value "data", "MC", or "any".
 * If "runType" is not provided, "any" is assumed.
 * The parameter "values" is only used when the URL is "python:".  In this case,
 the value for each
 * column will be taken from the values array and the configuration is expected
 to be independend of id.
 *
 * In any URL, envrironment variables in the format ${VARNAME} will be expanded.
 Also, the variable ${LDMX_CONDITION_TAG} will be replaced with the
 * tagname provided in the constructor.
 */
class SimpleCSVTableProvider : public framework::ConditionsObjectProvider {
 public:
  SimpleCSVTableProvider(const std::string& name, const std::string& tagname,
                         const framework::config::Parameters& parameters, framework::Process& process);

  virtual ~SimpleCSVTableProvider();

  virtual std::pair<const framework::ConditionsObject*, framework::ConditionsIOV> getCondition(
      const ldmx::EventHeader& context);

 private:
  enum { OBJ_unknown, OBJ_int, OBJ_double } objectType_;
  std::vector<std::string> columns_;

  struct Entry {
    framework::ConditionsIOV iov_;
    std::string url_;
    std::vector<int> ivalues_;
    std::vector<double> dvalues_;
  };

  std::vector<Entry> entries_;

  /**
   * Utility for expanding environment variables
   */
  std::string expandEnv(const std::string& s) const;
};

}  // namespace conditions
#endif
