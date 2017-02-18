#include "DetDescr/DetectorID.h"

namespace ldmx {

DetectorID::~DetectorID() {
    // Delete the field list and its objects as we assume this class owns it.
    for (IDField::IDFieldList::iterator it = fieldList_->begin(); it != fieldList_->end(); it++) {
        delete (*it);
    }
    delete fieldList_;
}

DetectorID::DetectorID() : rawValue_(0), fieldList_(0) {
}

DetectorID::DetectorID(IDField::IDFieldList* fieldList) : rawValue_(0) {
    setFieldList(fieldList);
    init();
}

DetectorID::RawValue DetectorID::getRawValue() {
    return rawValue_;
}

void DetectorID::setRawValue(RawValue rawValue) {
    this->rawValue_ = rawValue;
}

const DetectorID::FieldValueList& DetectorID::unpack() {
    std::fill(fieldValues_.begin(), fieldValues_.end(), 0);
    for (IDField::IDFieldList::iterator it = fieldList_->begin();
            it != fieldList_->end(); it++) {
        IDField* field = (*it);
        unsigned result = (field->getBitMask() & rawValue_) >> field->getStartBit();
        this->fieldValues_[field->getIndex()] = result;
    }
    return this->fieldValues_;
}

DetectorID::RawValue DetectorID::pack() {
    rawValue_ = 0;
    for (IDField::IDFieldList::iterator it = fieldList_->begin();
                it != fieldList_->end(); it++) {
        IDField* field = (*it);
        unsigned fieldValue = fieldValues_[field->getIndex()];
        rawValue_ = rawValue_ | (fieldValue << field->getStartBit());
    }
    return rawValue_;
}

DetectorID::FieldValue DetectorID::getFieldValue(int i) {
    return fieldValues_[i];
}

void DetectorID::setFieldValue(int i, FieldValue val) {
    fieldValues_[i] = val;
}

void DetectorID::setFieldValue(const std::string& fieldName, FieldValue fieldValue) {
    fieldValues_[fieldMap_[fieldName]->getIndex()] = fieldValue;
}

IDField::IDFieldList* DetectorID::getFieldList() {
    return fieldList_;
}

IDField* DetectorID::getField(const std::string& fieldName) {
    return fieldMap_[fieldName];
}

DetectorID::FieldValue DetectorID::getFieldValue(const std::string& fieldName) {
    return fieldValues_[fieldMap_[fieldName]->getIndex()];
}

void DetectorID::setFieldList(IDField::IDFieldList* fieldList) {
    // Set the list of fields.
    this->fieldList_ = fieldList;

    // Reinitialize data structures.
    init();
}

void DetectorID::init() {

    // Fill map of name to field info.
    fieldMap_.clear();
    for (IDField::IDFieldList::iterator it = fieldList_->begin(); it != fieldList_->end(); it++) {
        fieldMap_[(*it)->getFieldName()] = *it;
    }

    // Resize array to hold the correct number of field values.
    this->fieldValues_.resize(fieldList_->size());
}

}
