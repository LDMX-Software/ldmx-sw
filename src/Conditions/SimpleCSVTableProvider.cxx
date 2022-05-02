#include "Conditions/SimpleCSVTableProvider.h"
#include "Conditions/SimpleTableStreamers.h"
#include "Conditions/URLStreamer.h"

DECLARE_CONDITIONS_PROVIDER_NS(conditions, SimpleCSVTableProvider);

namespace conditions {

SimpleCSVTableProvider::~SimpleCSVTableProvider() {}

SimpleCSVTableProvider::SimpleCSVTableProvider(
    const std::string& name, const std::string& tagname,
    const framework::config::Parameters& parameters,
    framework::Process& process)
    : framework::ConditionsObjectProvider(name, tagname, parameters, process) {
  columns_ = parameters.getParameter<std::vector<std::string>>("columns");
  std::string dtype = parameters.getParameter<std::string>("dataType");
  if (dtype == "int" || dtype == "integer")
    objectType_ = SimpleCSVTableProvider::OBJ_int;
  if (dtype == "double" || dtype == "float")
    objectType_ = SimpleCSVTableProvider::OBJ_double;

  std::vector<framework::config::Parameters> plist =
      parameters.getParameter<std::vector<framework::config::Parameters>>(
          "entries");
  for (auto aprov : plist) {
    SimpleCSVTableProvider::Entry item;
    int firstRun = aprov.getParameter<int>("firstRun", -1);
    int lastRun = aprov.getParameter<int>("lastRun", -1);
    std::string rtype = aprov.getParameter<std::string>("runType", "any");
    bool isMC = (rtype == "any" || rtype == "MC");
    bool isData = (rtype == "any" || rtype == "data");
    item.iov_ = framework::ConditionsIOV(firstRun, lastRun, isData, isMC);
    item.url_ = aprov.getParameter<std::string>("URL");
    if (objectType_ == OBJ_int && aprov.exists("values")) {
      item.ivalues_ = aprov.getParameter<std::vector<int>>("values");
      if (item.ivalues_.size() != columns_.size()) {
        EXCEPTION_RAISE("ConditionsException",
                        "Mismatch in values vector (" +
                        std::to_string(item.ivalues_.size()) +
                        ") and columns vector (" +
                            std::to_string(columns_.size()) + ") in " +
                        getConditionObjectName());
      }
    }
    if (objectType_ == OBJ_double && aprov.exists("values")) {
      item.dvalues_ = aprov.getParameter<std::vector<double>>("values");
      if (item.dvalues_.size() != columns_.size()) {
        EXCEPTION_RAISE("ConditionsException",
                        "Mismatch in values vector (" +
                        std::to_string(item.dvalues_.size()) +
                        ") and columns vector (" +
                        std::to_string(columns_.size()) + ") in " +
                        getConditionObjectName());
      }
    }
    // here we check for overlaps
    
    for (auto tabledef : entries_) {
      if (item.iov_.overlaps(tabledef.iov_)) {
        std::stringstream err;
        err << "Table '" << getConditionObjectName()
            << "' has entries with overlapping providers " << tabledef.url_
            << " and " << item.url_;
        EXCEPTION_RAISE("ConditionsException", err.str());
      }
    }
    // add to the list
    entries_.push_back(item);
  }
}

std::string SimpleCSVTableProvider::expandEnv(const std::string& s) const {
  std::string retval;
  std::string::size_type j = 0;
  for (std::string::size_type i = s.find("${", j); i < s.size();
       i = s.find("${", j)) {
    if (i != j) retval.append(s, j, i - j);  // append
    j = s.find("}", i);                      // look for the end of request
    std::string key = std::string(s, i + 2, j - i - 2);
    if (key == "LDMX_CONDITION_TAG")
      retval += getTagName();
    else {
      const char* cenv = getenv(key.c_str());
      if (cenv != 0) {
        retval += cenv;
      } else {
        // ERROR!
      }
    }
    j++;
  }
  if (j < s.size()) retval.append(s, j);
  //	std::cout << s << "=>" << retval << std::endl;
  return retval;
}

std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
SimpleCSVTableProvider::getCondition(const ldmx::EventHeader& context) {
  for (auto tabledef : entries_) {
    //	    std::cout << condition_name << " " << tabledef.objectName_ << " " <<
    // tabledef.iov_ << " " << std::endl;
    if (tabledef.iov_.validForEvent(context)) {
      std::string expurl = expandEnv(tabledef.url_);

      if (expurl == "python:") {
        // here we just copy values...
        if (objectType_ == OBJ_int) {
          IntegerTableCondition* table =
              new IntegerTableCondition(getConditionObjectName(), columns_);
          table->setIdMask(0);  // all ids are the same...
          table->add(0, tabledef.ivalues_);
          return std::pair<const framework::ConditionsObject*,
                           framework::ConditionsIOV>(table, tabledef.iov_);
        } else if (objectType_ == OBJ_double) {
          conditions::DoubleTableCondition* table =
              new conditions::DoubleTableCondition(getConditionObjectName(),
                                                   columns_);
          table->setIdMask(0);  // all ids are the same...
          table->add(0, tabledef.dvalues_);
          return std::pair<const framework::ConditionsObject*,
                           framework::ConditionsIOV>(table, tabledef.iov_);
        }
      } else {
        std::unique_ptr<std::istream> stream=urlstream(expurl);
        
        if (objectType_ == OBJ_int) {
          IntegerTableCondition* table =
              new IntegerTableCondition(getConditionObjectName(), columns_);
          conditions::utility::SimpleTableStreamerCSV::load(*table, *(stream.get()));
          return std::pair<const framework::ConditionsObject*,
                           framework::ConditionsIOV>(table, tabledef.iov_);
        } else if (objectType_ == OBJ_double) {
          conditions::DoubleTableCondition* table =
              new conditions::DoubleTableCondition(getConditionObjectName(),
                                                   columns_);
          conditions::utility::SimpleTableStreamerCSV::load(*table, *(stream.get()));
          return std::pair<const framework::ConditionsObject*,
                           framework::ConditionsIOV>(table, tabledef.iov_);
        }
      }
    }
  }
  return std::pair<const framework::ConditionsObject*,
                   framework::ConditionsIOV>(0, framework::ConditionsIOV());
}

}  // namespace conditions
