#include "Conditions/SimpleTableStreamers.h"
#include <algorithm>
#include <iomanip>
#include "DetDescr/DetectorIDInterpreter.h"
#include "boost/format.hpp"

namespace ldmx {
namespace utility {

static void storeIdFields(unsigned int id, std::ostream& s) {
  DetectorIDInterpreter did(id);

  for (auto field : did.getFieldList()) {
    s << ",id:\"" << field->getFieldName() << '"';
  }
}

template <class T, class V>
void storeT(const T& t, std::ostream& s, bool expandIds) {
  char buffer[100];
  // write the header line
  s << "\"DetID\"";
  if (expandIds && t.getRowCount() > 0) {
    storeIdFields(t.getRowId(0), s);
  }
  for (auto name : t.getColumnNames()) {
    s << ",\"" << name << "\"";
  }
  s << std::endl;
  // write the data rows
  for (unsigned int irow = 0; irow < t.getRowCount(); irow++) {
    std::pair<unsigned int, std::vector<V> > row = t.getRow(irow);
    // write the id in hex
    s << boost::format("0x%08x") % row.first;
    if (expandIds) {
      DetectorIDInterpreter did(row.first);

      for (int i = 0; i < did.getFieldCount(); i++)
        s << ',' << std::setprecision(10) << did.getFieldValue(i);
    }
    for (auto col : row.second) s << ',' << col;
    s << std::endl;
  }
}
void SimpleTableStreamerCSV::store(const IntegerTableCondition& t,
                                   std::ostream& s, bool expandIds) {
  storeT<IntegerTableCondition, int>(t, s, expandIds);
}
void SimpleTableStreamerCSV::store(const DoubleTableCondition& t,
                                   std::ostream& s, bool expandIds) {
  storeT<DoubleTableCondition, double>(t, s, expandIds);
}

static int convert(const std::string& s, int dummy) {
  return strtol(s.c_str(), 0, 0);
}

static double convert(const std::string& s, double dummy) {
  return atof(s.c_str());
}

static std::vector<std::string> splitCSV(const std::string& s) {
  std::vector<std::string> rv;
  std::string field;
  bool inquote = false;
  for (auto chr : s) {
    if (chr == '"') {
      inquote = !inquote;
    } else if ((chr == ',' || chr == '\t') && !inquote) {
      if (!field.empty()) {
        //			std::cout << "Field: '" << field << "'\n";
        rv.push_back(field);
      }
      field.clear();
    } else if (isspace(chr) && !inquote) {  // do not add spaces
    } else if (chr == '#' && !inquote) {
      break;  // comment
    } else {
      field += chr;
    }
  }
  if (!field.empty()) {
    rv.push_back(field);
    //		std::cout << "Field: '" << field << "'\n";
  }
  return rv;
}
static std::vector<std::string>::const_iterator find(
    const std::vector<std::string>& v, const std::string& a) {
  for (auto i = v.begin(); i != v.end(); i++) {
    if ((*i) == a) return i;
  }
  return v.end();
}

template <class T, class V>
void loadT(T& table, std::istream& is) {
  table.clear();
  // first work with the header line
  std::string line;
  std::vector<std::string> split;
  int iDetID = -1;
  int iline = 0;
  size_t ncolin;

  while (!is.eof()) {
    iline++;
    std::getline(is, line);
    split = splitCSV(line);
    if (split.size() < 1) {
      split.clear();
      continue;  // need at least an id column and as many columns as requested
    }
    if (find(split, "DetID") == split.end() &&
        find(split, "subdetector") == split.end()) {
      EXCEPTION_RAISE("ConditionsException",
                      "Malformed CSV file with no DetId or subdetector column");
    } else {
      break;
    }
  }
  if (is.eof()) {
    EXCEPTION_RAISE("ConditionsException", "CSV file has no valid header");
  }
  // ok, we have a header line.  Do we have a DetID column?
  std::vector<std::string>::const_iterator id = find(split, "DetID");
  if (id != split.end()) {
    // good this is simpler...
    iDetID = int(id - split.begin());
  } else {
    EXCEPTION_RAISE("MissingFeatureException",
                    "Cannot actually load CSV file without valid DetID column "
                    "at this point");
  }
  ncolin = split.size();
  // check for columns which match all those requested in the table
  std::map<unsigned int, int> table_to_csv;
  for (unsigned int ic = 0; ic != table.getColumnCount(); ic++) {
    auto fc = find(split, table.getColumnName(ic));
    if (fc == split.end()) {
      EXCEPTION_RAISE(
          "ConditionsException",
          "Missing column '" + table.getColumnName(ic) + "' in CSV table load");
    }
    table_to_csv[ic] = int(fc - split.begin());
  }

  // processing additional lines
  while (!is.eof()) {
    iline++;
    std::getline(is, line);
    split = splitCSV(line);
    if (split.size() == 0) continue;  // ignore comment lines
    if (split.size() != ncolin) {
      EXCEPTION_RAISE("ConditionsException", "Mismatched number of columns (" +
                                                 std::to_string(split.size()) +
                                                 "!=" + std::to_string(ncolin) +
                                                 ") on line " +
                                                 std::to_string(iline));
    }
    unsigned int id(0);
    std::vector<V> values(table_to_csv.size(), 0);
    if (iDetID >= 0) id = strtoul(split[iDetID].c_str(), 0, 0);
    V dummy(0);
    for (auto icopy : table_to_csv) {
      values[icopy.first] = convert(split[icopy.second], dummy);
    }
    if (id != 0) {
      table.add(id, values);
    }
  }
}

void SimpleTableStreamerCSV::load(IntegerTableCondition& table,
                                  std::istream& is) {
  loadT<IntegerTableCondition, int>(table, is);
}

void SimpleTableStreamerCSV::load(DoubleTableCondition& table,
                                  std::istream& is) {
  loadT<DoubleTableCondition, double>(table, is);
}
}  // namespace utility
}  // namespace ldmx
