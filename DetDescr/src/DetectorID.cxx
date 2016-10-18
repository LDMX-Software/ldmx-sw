#include "DetDescr/DetectorID.h"

namespace detdescr {

DetectorID::~DetectorID() {
    // Delete the field list and its objects as we assume this class owns it.
    for (IDField::IDFieldList::iterator it = idFieldList->begin(); it != idFieldList->end(); it++) {
        delete (*it);
    }
    delete idFieldList;
}

DetectorID::DetectorID() : rawValue(0), idFieldList(0) {
}

DetectorID::DetectorID(IDField::IDFieldList* idFieldList) : rawValue(0) {

    // Set the list of fields.
    this->idFieldList = idFieldList;

    // Fill map of name to field info.
    for (IDField::IDFieldList::iterator it = idFieldList->begin(); it != idFieldList->end(); it++) {
        idFieldMap[(*it)->getFieldName()] = *it;
    }

    // Resize value array for correct number of fields.
    this->values.resize(idFieldList->size());
}

DetectorID::RawValue DetectorID::getRawValue() {
    return rawValue;
}

void DetectorID::setRawValue(RawValue rawValue) {
    this->rawValue = rawValue;
}

const DetectorID::FieldValueList& DetectorID::unpack() {
    std::fill(values.begin(), values.end(), 0);
    for (IDField::IDFieldList::iterator it = idFieldList->begin();
            it != idFieldList->end(); it++) {
        IDField* field = (*it);
        unsigned result = (field->getBitMask() & rawValue) >> field->getStartBit();
        this->values[field->getIndex()] = result;
    }
    return this->values;
}

DetectorID::RawValue DetectorID::pack() {
    rawValue = 0;
    for (IDField::IDFieldList::iterator it = idFieldList->begin();
                it != idFieldList->end(); it++) {
        IDField* field = (*it);
        unsigned fieldValue = values[field->getIndex()];
        rawValue = rawValue | (fieldValue << field->getStartBit());
    }
    return rawValue;
}

DetectorID::FieldValue DetectorID::getFieldValue(int i) {
    return values[i];
}

void DetectorID::setFieldValue(int i, FieldValue val) {
    values[i] = val;
}

void DetectorID::setFieldValue(const std::string& fieldName, FieldValue fieldValue) {
    values[idFieldMap[fieldName]->getIndex()] = fieldValue;
}

IDField::IDFieldList* DetectorID::getFieldList() {
    return idFieldList;
}

IDField* DetectorID::getField(const std::string& fieldName) {
    return idFieldMap[fieldName];
}

DetectorID::FieldValue DetectorID::getFieldValue(const std::string& fieldName) {
    return values[idFieldMap[fieldName]->getIndex()];
}

}
