#ifndef PARAMETERSET_H_
#define PARAMETERSET_H_

#include <map>
#include <string>
#include <vector>

namespace ldmxsw {

    /** Class which contains the parameters for an EventProcessor configuration 
     */
    class ParameterSet {
    public:
	
	/** 
	 * Get an integer by name or throw an exception if not available, or not the right type
	 * @param name Name of the integer
	*/
        int getInteger(const std::string& name) const;
	
	/**
	 * Get an integer by name or use the given default if not available.  If the wrong type, throw an exception.
	 * @param name Name of the integer
	 * @param defaultValue Default value to ue if the parameter was not provided
	*/
        int getInteger(const std::string& name, int defaultValue) const;

	/** 
	 * Get a double by name or throw an exception if not available, or not the right type
	 * @param name Name of the double parameter
	*/
        double getDouble(const std::string& name) const;

	/**
	 * Get a double by name or use the given default if not available.  If the wrong type, throw an exception.
	 * @param name Name of the double parameter
	 * @param defaultValue Default value to ue if the parameter was not provided
	*/
        double getDouble(const std::string& name, double defaultValue) const;

	/** 
	 * Get a string by name or throw an exception if not available, or not the right type
	 * @param name Name of the string parameter
	*/
        const std::string& getString(const std::string& name) const;

	/**
	 * Get a string by name or use the given default if not available.  If the wrong type, throw an exception.
	 * @param name Name of the string parameter
	 * @param defaultValue Default value to ue if the parameter was not provided
	*/
        const std::string& getString(const std::string& name, const std::string& defaultValue) const;
    
	/** 
	 * Get a vector of integers by name or throw an exception if not available, or not the right type
	 * @param name Name of the vector of integers
	*/
	const std::vector<int>& getVInteger(const std::string& name) const;

	/**
	 * Get a vector of integers by name or use the given default if not available.  If the wrong type, throw an exception.
	 * @param name Name of the vector of integers
	 * @param defaultValue Default value to ue if the parameter was not provided
	*/
        const std::vector<int>& getVInteger(const std::string& name, const std::vector<int>& defaultValue) const;

	/** 
	 * Get a vector of doubles by name or throw an exception if not available, or not the right type
	 * @param name Name of the vector of doubles
	*/
        const std::vector<double>& getVDouble(const std::string& name) const;

	/**
	 * Get a vector of doubles by name or use the given default if not available.  If the wrong type, throw an exception.
	 * @param name Name of the vector of doubles
	 * @param defaultValue Default value to ue if the parameter was not provided
	*/
        const std::vector<double>& getVDouble(const std::string& name, const std::vector<double>& defaultValue) const;

	/** 
	 * Get a vector of strings by name or throw an exception if not available, or not the right type
	 * @param name Name of the vector of strings
	*/
        const std::vector<std::string>& getVString(const std::string& name) const;

	/**
	 * Get a vector of strings by name or use the given default if not available.  If the wrong type, throw an exception.
	 * @param name Name of the vector of strings
	 * @param defaultValue Default value to ue if the parameter was not provided
	*/
        const std::vector<std::string>& getVString(const std::string& name, const std::vector<std::string>& defaultValue) const;
        
	/** Implementation to add an integer to the ParameterSet */
        void insert(const std::string& name, int value);
	/** Implementation to add a double to the ParameterSet */
        void insert(const std::string& name, double value);
	/** Implementation to add a string to the ParameterSet */
        void insert(const std::string& name, const std::string& value);
	/** Implementation to add a vector of integers to the ParameterSet */
        void insert(const std::string& name, const std::vector<int>& values);
	/** Implementation to add a vector of doubles to the ParameterSet */
        void insert(const std::string& name, const std::vector<double>& values);
	/** Implementation to add a vector of strings to the ParameterSet */
        void insert(const std::string& name, const std::vector<std::string>& values);    

    private:
    
        typedef enum { et_NoType, et_Integer, et_Double, et_String, et_VInteger, et_VDouble, et_VString, et_ParameterSet } ElementType;
        struct Element {
            Element() : et_{et_NoType} { }
            Element(int inval) : et_{et_Integer}, intval_{inval} { }
            Element(double inval) : et_{et_Double}, doubleval_{inval} { }
            Element(const std::string& inval) : et_{et_String}, strval_{inval} { }
            Element(const std::vector<int>& inval);
            Element(const std::vector<double>& inval);
            Element(const std::vector<std::string>& inval);
    
            ElementType et_;
            int intval_;
            double doubleval_;
            std::string strval_;
            std::vector<int> ivecVal_;
            std::vector<double> dvecVal_;
            std::vector<std::string> svecVal_;
            ParameterSet* subsetVal_{0};      
        };
        std::map<std::string,Element> elements_;
};
  
}

#endif
