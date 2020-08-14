/**
 * @file SimpleTableStreamers
 * @brief Utility classes for streaming SimpleTableConditions in various formats
 * @author Jeremy Mans, University of Minnesota
 */
#ifndef FRAMEWORK_SIMPLETABLESTREAMERS_H_
#define FRAMEWORK_SIMPLETABLESTREAMERS_H_

#include <iostream>

#include "Conditions/SimpleTableCondition.h"


namespace ldmx {
    
    // using an internal namespace for these support classes to avoid any clashes
    namespace utility {

	/**
	 * @class Converts a simple table to/from a CSV file
	 */
	class SimpleTableStreamerCSV {
	    public:

	    /** 
	     * Convert the table into a stream
	     */
	    static void store(const IntegerTableCondition&, std::ostream&, bool expandIds=true);
	    static void store(const DoubleTableCondition&, std::ostream&, bool expandIds=true);
	    /** Load the table from a stream
	     * Columns must be defined by the user.  Matching columns from the CSV file will be copied in.
	     * The first row of the table must be the column headers.
	     * If the DetID column is not present or zero, the loader will attempt to use the "subdetector" column to determine which other columns should be used to construct a detector id using the interpreter.
	     */
	    static void load(IntegerTableCondition&, std::istream&);
	    static void load(DoubleTableCondition&, std::istream&);
	};
    }
}


#endif 
