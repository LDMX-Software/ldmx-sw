#include "Framework/ParameterSet.h"
#include "Framework/Exception.h"

namespace ldmxsw {

void ParameterSet::insert(const std::string& name, int value) {
    elements_[name] = Element(value);
}

int ParameterSet::getInteger(const std::string& name) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        EXCEPTION_RAISE("ParameterNotFound", "Integer parameter '" + name + "' not found");
    }
    if (ptr->second.et_ != et_Integer) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not an integer");
    }
    return ptr->second.intval_;
}

int ParameterSet::getInteger(const std::string& name, int defaultValue) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        return defaultValue;
    }
    if (ptr->second.et_ != et_Integer) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not an integer");
    }
    return ptr->second.intval_;
}

void ParameterSet::insert(const std::string& name, double value) {
    elements_[name] = Element(value);
}

double ParameterSet::getDouble(const std::string& name) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        EXCEPTION_RAISE("ParameterNotFound", "Double parameter '" + name + "' not found");
    }
    if (ptr->second.et_ != et_Double) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not a double");
    }
    return ptr->second.doubleval_;
}

double ParameterSet::getDouble(const std::string& name, double defaultValue) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        return defaultValue;
    }
    if (ptr->second.et_ != et_Double) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not a double");
    }
    return ptr->second.doubleval_;
}

void ParameterSet::insert(const std::string& name, const std::string& value) {
    elements_[name] = Element(value);
}

const std::string& ParameterSet::getString(const std::string& name) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        EXCEPTION_RAISE("ParameterNotFound", "String parameter '" + name + "' not found");
    }
    if (ptr->second.et_ != et_String) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not a string");
    }
    return ptr->second.strval_;
}

const std::string& ParameterSet::getString(const std::string& name, const std::string& defaultValue) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        return defaultValue;
    }
    if (ptr->second.et_ != et_String) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not a string");
    }
    return ptr->second.strval_;
}

/* --------------- Vectors of integers ------------------------*/
ParameterSet::Element::Element(const std::vector<int>& inval) :
        et_ { ParameterSet::et_VInteger }, ivecVal_(inval) {
}

void ParameterSet::insert(const std::string& name, const std::vector<int>& values) {
    elements_[name] = Element(values);
}

const std::vector<int>& ParameterSet::getVInteger(const std::string& name) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        EXCEPTION_RAISE("ParameterNotFound", "Parameter '" + name + "' not found");
    }
    if (ptr->second.et_ != et_VInteger) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not a vector of integers");
    }
    return ptr->second.ivecVal_;
}

const std::vector<int>& ParameterSet::getVInteger(const std::string& name, const std::vector<int>& defaultValue) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        return defaultValue;
    }
    if (ptr->second.et_ != et_VInteger) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not a vector of integers");
    }
    return ptr->second.ivecVal_;
}

/* --------------- Vectors of doubles ------------------------*/
ParameterSet::Element::Element(const std::vector<double>& inval) :
        et_ { ParameterSet::et_VDouble }, dvecVal_(inval) {
}

void ParameterSet::insert(const std::string& name, const std::vector<double>& values) {
    elements_[name] = Element(values);
}

const std::vector<double>& ParameterSet::getVDouble(const std::string& name) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        EXCEPTION_RAISE("ParameterNotFound", "Parameter '" + name + "' not found");
    }
    if (ptr->second.et_ != et_VDouble) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not a vector of doubles");
    }
    return ptr->second.dvecVal_;
}

const std::vector<double>& ParameterSet::getVDouble(const std::string& name, const std::vector<double>& defaultValue) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        return defaultValue;
    }
    if (ptr->second.et_ != et_VDouble) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not a vector of doubles");
    }
    return ptr->second.dvecVal_;
}

/* --------------- Vectors of strings ------------------------*/
ParameterSet::Element::Element(const std::vector<std::string>& inval) :
        et_ { ParameterSet::et_VString }, svecVal_(inval) {
}

void ParameterSet::insert(const std::string& name, const std::vector<std::string>& values) {
    elements_[name] = Element(values);
}

const std::vector<std::string>& ParameterSet::getVString(const std::string& name) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        EXCEPTION_RAISE("ParameterNotFound", "Parameter '" + name + "' not found");
    }
    if (ptr->second.et_ != et_VString) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not a vector of strings");
    }
    return ptr->second.svecVal_;
}
const std::vector<std::string>& ParameterSet::getVString(const std::string& name, const std::vector<std::string>& defaultValue) const {
    std::map<std::string, Element>::const_iterator ptr = elements_.find(name);
    if (ptr == elements_.end()) {
        return defaultValue;
    }
    if (ptr->second.et_ != et_VString) {
        EXCEPTION_RAISE("ParameterTypeError", "Parameter '" + name + "' is not a vector of strings");
    }
    return ptr->second.svecVal_;
}

}
