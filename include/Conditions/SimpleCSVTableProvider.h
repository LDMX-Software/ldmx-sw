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

 * The mapping of CSV files to IOVs can be provided either through the python code 
 * (appropriate mostly for rather small sets of conditions information) or through 
 * a CSV file which indentifies the mapping.
 *
 * The "entries" array in python is an array of parameter sets used for direct configuration from python.
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
 * The "entriesURL" is a URL pointing to a CSV file containing the following columns:
 * "FIRST_RUN" which is an integer column indicating the first run of the IOV, or -1 for no lower limit
 * "LAST_RUN" which is an integer column indicating the last run of the IOV, or -1 for no upper limit
 * "RUNTYPE" which is a string column with the expected values "data", "MC" or "any".  This column is not case-sensitive.
 * "URL" which is a string column indicating the location of the table.
 *
 * In any URL, envrironment variables in the format ${VARNAME} will be expanded.
 * In addition, there are some special 'environment' variables: 
 *  * ${LDMX_CONDITION_BASEURL} will be replaced with
 *  the parameter 'condition_baseurl' set in the python configuration.
 *  * ${LDMX_CONDITION_TAG} will be replaced with the
 *  tagname provided in the constructor.
 */
class SimpleCSVTableProvider : public framework::ConditionsObjectProvider {
 public:
  SimpleCSVTableProvider(const std::string& name, const std::string& tagname,
                         const framework::config::Parameters& parameters,
                         framework::Process& process);

  virtual ~SimpleCSVTableProvider();

  virtual std::pair<const framework::ConditionsObject*,
                    framework::ConditionsIOV>
  getCondition(const ldmx::EventHeader& context);

 private:
  enum { OBJ_unknown, OBJ_int, OBJ_double } objectType_;
  std::vector<std::string> columns_;
  std::string entriesURL_;
  std::string conditions_baseURL_;

  struct Entry {
    framework::ConditionsIOV iov_;
    std::string url_;
    std::vector<int> ivalues_; // values for python-set common (channel-independent) parameters
    std::vector<double> dvalues_; // values for python-set common (channel-independent) parameters
  };

  std::vector<Entry> entries_;

  /**
   * Utility for expanding environment variables
   */
  std::string expandEnv(const std::string& s) const;
};

}  // namespace conditions
#endif
