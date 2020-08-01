#include "Conditions/SimpleTableStreamers.h"
#include "boost/format.hpp"
#include "DetDescr/DetectorIDInterpreter.h"
#include <iomanip>
#include <algorithm>

namespace ldmx {
    namespace utility {    

	static void storeIdFields(unsigned int id, std::ostream& s) {
	    DetectorIDInterpreter did(id);

	    for (auto field : did.getFieldList()) {
		s << ",\"" << field->getFieldName() << '"';
	    }
	}
    
	void SimpleTableStreamerCSV::store(const IntegerTableCondition& t, std::ostream& s, bool expandIds) {
	    char buffer[100];
	    // write the header line
	    s << "\"DetID\"";
	    if (expandIds && t.getRowCount()>0) {
		storeIdFields(t.getRowId(0),s);
	    }
	    for (auto name: t.getColumnNames()) {
		s << ",\"" << name << "\"";
	    }
	    s << std::endl;
	    // write the data rows
	    for (unsigned int irow=0; irow<t.getRowCount(); irow++) {
		std::pair<unsigned int, std::vector<int> > row = t.getRow(irow);
		// write the id in hex
		s << boost::format("0x%08x") % row.first;
		if (expandIds) {
		    DetectorIDInterpreter did(row.first);

		    for (int i=0; i<did.getFieldCount(); i++) 
			s << ',' << did.getFieldValue(i);
		}
		for (auto col : row.second) 
		    s << ',' << col;      
		s << std::endl;      
	    }
	}
	void SimpleTableStreamerCSV::store(const DoubleTableCondition& t, std::ostream& s, bool expandIds) {
	    char buffer[100];
	    // write the header line
	    s << "\"DetId\"";
	    if (expandIds && t.getRowCount()>0) {
		storeIdFields(t.getRowId(0),s);
	    }
	    for (auto name: t.getColumnNames()) {
		s << ",\"" << name << "\"";
	    }
	    s << std::endl;
	    // write the data rows
	    for (unsigned int irow=0; irow<t.getRowCount(); irow++) {
		std::pair<unsigned int, std::vector<double> > row = t.getRow(irow);
		// write the id in hex
		s << boost::format("0x%08x") % row.first;
		if (expandIds) {
		    DetectorIDInterpreter did(row.first);

		    for (int i=0; i<did.getFieldCount(); i++) 
			s << ',' << std::setprecision(10) <<  did.getFieldValue(i);
		}
		for (auto col : row.second) 
		    s << ',' << col;      
		s << std::endl;      
	    }
	}
 
      
	static std::vector<std::string> splitCSV(const std::string& s) {
	    std::vector<std::string> rv;
	    std::string field;
	    bool inquote=false;
	    for (auto chr : s ) {
		if (chr=='"') {
		    inquote=!inquote;
		} else if ((chr==',' || chr=='\t') && !inquote) {
		    if (!field.empty()) rv.push_back(field);
		    field.clear();
		} else if (isspace(chr) && !inquote) { // do not add spaces		    
		} else if (chr=='#' && !inquote) {
		    break; // comment
		} else {
		    field+=chr;
		}	
	    }
	    if (!field.empty()) rv.push_back(field);
	    return rv;
	}
	static std::vector<std::string>::const_iterator find(const std::vector<std::string>& v, const std::string& a) {
	    for (auto i = v.begin(); i!=v.end(); i++) {
		if ((*i)==a) return i;
	    }
	    return v.end();
	}
    
	void SimpleTableStreamerCSV::load(IntegerTableCondition& table, std::istream& is) {
	    table.clear();
	    // first work with the header line
	    std::string line;
	    std::vector<std::string> split;
	    int iDetID=-1;

	    while (!is.eof()) {
		std::getline(is,line);
		split=splitCSV(line);
		if (split.size()<1+table.getColumnCount()) {
		    split.clear();
		    continue; // need at least an id column and as many columns as requested
		}
		if (find(split,"DetID")==split.end() && find(split,"subdetector")==split.end()) {
		    EXCEPTION_RAISE("ConditionsException","Malformed CSV file with no DetId or subdetector column");
		}
	    }
	    if (is.eof()) {
		EXCEPTION_RAISE("ConditionsException","CSV file has no valid header");
	    }
	    // ok, we have a header line.  Do we have a DetID column?
	    std::vector<std::string>::const_iterator id=find(split,"DetID");
	    if (id!=split.end()) {
		// good this is simpler...
		iDetID=int(id-split.begin());
	    }
	}
    
    }  
}

