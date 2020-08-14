/**
 * @file ConditionsObjectProvider which can be configured to load conditions objects from
 *  CSV files provided via URLs with IOVs defined in the configuration.  Configuration can be
 * either inline or via an external CSV file.
 */
#ifndef CONDITIONS_SIMPLECSVTABLEPROVIDER_H_
#define CONDITIONS_SIMPLECSVTABLEPROVIDER_H_

#include "Framework/ConditionsObjectProvider.h"

namespace ldmx {

    /**
     * @class ConditionsObjectProvider which can be configured to load conditions objects from
     *  CSV files.
     *
     * Configuration via Python uses a parameter "provides" which is a list of elements each of which must contain
     * "name","dataType", "columns", and "URL".  The parameter set _may_ contain "firstRun", "lastRun", and "runType".
     * "DataType may be "int" or "double".  The "columns" parameter is the list of names of data columns to be read out.
     * If "firstRun" is not provided, -1 is assumed.  If "lastRun" is not provided, -1 is assumed.
     * The parameter "runType" could have the value "data", "MC", or "any".
     * If "runType" is not provided, "any" is assumed.
     * 
     * In any URL, envrironment variables in the format ${VARNAME} will be expanded.  Also, the variable ${LDMX_CONDITION_TAG} will be replaced with the 
     * tagname provided in the constructor.
     */
    class SimpleCSVTableProvider : public ConditionsObjectProvider {
	public:
	
	SimpleCSVTableProvider(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process);

	virtual ~SimpleCSVTableProvider();

	virtual std::pair<const ConditionsObject*,ConditionsIOV> getCondition(const std::string& condition_name, const EventHeader& context);

        static void runTest(const std::string& host, const std::string& path, std::string& ss);
	
	private:
	
	struct Item {
	    std::string objectName_;
	    enum { OBJ_unknown, OBJ_int, OBJ_double } objectType_;
	    ConditionsIOV iov_;
	    std::string url_;
	    std::vector<std::string> columns_;
	};
	std::vector<Item> items_;

	/** 
	 * Utility for expanding environment variables
	 */
	std::string expandEnv(const std::string& s) const;
    };
}
#endif
