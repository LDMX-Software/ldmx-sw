#include "DetDescr/DetectorIDInterpreter.h"
#include "DetDescr/HcalID.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/TrigScintID.h"
#include "DetDescr/TrackerID.h"

namespace ldmx {

    DetectorIDInterpreter::~DetectorIDInterpreter()  {
    }
    DetectorIDInterpreter::DetectorIDInterpreter() : id_(), p_fieldInfo_(0) {
	init();
    }

    DetectorIDInterpreter::DetectorIDInterpreter(DetectorID did) : id_(did), p_fieldInfo_(0) {
	init();
	unpack();
    }

    void DetectorIDInterpreter::setRawValue(DetectorID rawValue) {	
        id_=rawValue;
	init();
	unpack();
    }

    void DetectorIDInterpreter::unpack() {	
        std::fill(fieldValues_.begin(), fieldValues_.end(), 0);
	if (!p_fieldInfo_) return;
        for (auto field : p_fieldInfo_->fieldList_) {
            unsigned result = (field->getBitMask() & id_.raw()) >> field->getStartBit();
            this->fieldValues_[field->getIndex()] = result;
        }
    }

    void DetectorIDInterpreter::pack() {
	DetectorID::RawValue rawValue=0;
        for (auto field : p_fieldInfo_->fieldList_) {
            unsigned fieldValue = fieldValues_[field->getIndex()];
            rawValue = rawValue | ((fieldValue << field->getStartBit()) & field->getBitMask());
        }
	id_.setRawValue(rawValue);
    }

    DetectorIDInterpreter::FieldValue DetectorIDInterpreter::getFieldValue(int i) const {
        IDField* field = p_fieldInfo_->fieldList_.at(i);
        unsigned result = (field->getBitMask() & id_.raw()) >> field->getStartBit();
        return result;
    }

    void DetectorIDInterpreter::setFieldValue(int i, FieldValue val) {
        fieldValues_[i] = val;
	pack(); // keep packed
    }

    void DetectorIDInterpreter::setFieldValue(const std::string& fieldName, FieldValue fieldValue) {
	auto byname=p_fieldInfo_->fieldMap_.find(fieldName);
	if (byname!=p_fieldInfo_->fieldMap_.end()) fieldValues_[byname->second->getIndex()] = fieldValue;
	pack(); // keep packed
    }

    const IDField* DetectorIDInterpreter::getField(const std::string& fieldName) const {
	auto byname=p_fieldInfo_->fieldMap_.find(fieldName);
	if (byname!=p_fieldInfo_->fieldMap_.end()) return (byname->second);
	return 0;
    }

    DetectorIDInterpreter::FieldValue DetectorIDInterpreter::getFieldValue(const std::string& fieldName) const {
	auto byname=p_fieldInfo_->fieldMap_.find(fieldName);
        return getFieldValue(byname->second->getIndex());
    }

    void DetectorIDInterpreter::init() {
	if (g_rosettaStone.empty()) loadStandardInterpreters();

	p_fieldInfo_=0;

	if (id_.null()) return;	
	
	auto ptr = g_rosettaStone.find(id_.subdet());

	if (ptr == g_rosettaStone.end()) {
	    // use the generic id
	} else {
	    p_fieldInfo_=&(ptr->second);
	}
	
        this->fieldValues_.resize(p_fieldInfo_->fieldList_.size());
    }
    
    void DetectorIDInterpreter::registerInterpreter(SubdetectorIDType idtype,  const IDField::IDFieldList& fieldList) {
	assert(g_rosettaStone.find(idtype)==g_rosettaStone.end()); // no replacements
	SubdetectorIDFields fields;
	fields.fieldList_=fieldList;
	for (auto it : fieldList)
	    fields.fieldMap_[it->getFieldName()]=it;
	
    }

    void DetectorIDInterpreter::loadStandardInterpreters() {
	if (!g_rosettaStone.empty()) return;
	EcalID::createInterpreters();
	HcalID::createInterpreters();
	TrackerID::createInterpreters();
	TrigScintID::createInterpreters();
    }
}
