#ifndef ParameterSet_h_included
#define ParameterSet_h_included 1

#include <map>
#include <string>
#include <vector>

namespace ldmxsw {

  class ParameterSet {
  public:

    int getInteger(const std::string& name) const;
    int getInteger(const std::string& name, int defaultValue) const;
    double getDouble(const std::string& name) const;
    double getDouble(const std::string& name, double defaultValue) const;
    const std::string& getString(const std::string& name) const;
    const std::string& getString(const std::string& name, const std::string& defaultValue) const;

    const std::vector<int>& getVInteger(const std::string& name) const;
    const std::vector<int>& getVInteger(const std::string& name, const std::vector<int>& defaultValue) const;
    const std::vector<double>& getVDouble(const std::string& name) const;
    const std::vector<double>& getVDouble(const std::string& name, const std::vector<double>& defaultValue) const;
    const std::vector<std::string>& getVString(const std::string& name) const;
    const std::vector<std::string>& getVString(const std::string& name, const std::vector<std::string>& defaultValue) const;
    
    void insert(const std::string& name, int value);
    void insert(const std::string& name, double value);
    void insert(const std::string& name, const std::string& value);
    void insert(const std::string& name, const std::vector<int>& values);
    void insert(const std::string& name, const std::vector<double>& values);
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

#endif // ParameterSet_h_included
