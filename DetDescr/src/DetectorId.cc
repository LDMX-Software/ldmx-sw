#include "DetDescr/DetectorId.h"

DetectorId::~DetectorId() {
    // Delete the field list and its objects as we assume this class owns it.
    for (IdField::IdFieldList::iterator it = idFieldList->begin(); it != idFieldList->end(); it++) {
        delete (*it);
    }
    delete idFieldList;
}

DetectorId::DetectorId() : rawValue(0), idFieldList(0) {
}

DetectorId::DetectorId(IdField::IdFieldList* idFieldList) : rawValue(0) {

    // Set the list of fields.
    this->idFieldList = idFieldList;

    // Fill map of name to field info.
    for (IdField::IdFieldList::iterator it = idFieldList->begin(); it != idFieldList->end(); it++) {
        idFieldMap[(*it)->getFieldName()] = *it;
    }

    // Resize value array for correct number of fields.
    this->values.resize(idFieldList->size());
}

DetectorId::RawValue DetectorId::getRawValue() {
    return rawValue;
}

void DetectorId::setRawValue(RawValue rawValue) {
    this->rawValue = rawValue;
}

const DetectorId::FieldValueList& DetectorId::unpack() {
    std::fill(values.begin(), values.end(), 0);
    for (IdField::IdFieldList::iterator it = idFieldList->begin();
            it != idFieldList->end(); it++) {
        IdField* field = (*it);
        unsigned result = (field->getBitMask() & rawValue) >> field->getStartBit();
        this->values[field->getIndex()] = result;
    }
    return this->values;
}

DetectorId::RawValue DetectorId::pack() {
    rawValue = 0;
    for (IdField::IdFieldList::iterator it = idFieldList->begin();
                it != idFieldList->end(); it++) {
        IdField* field = (*it);
        unsigned fieldValue = values[field->getIndex()];
        rawValue = rawValue | (fieldValue << field->getStartBit());
    }
    return rawValue;
}

DetectorId::FieldValue DetectorId::getFieldValue(int i) {
    return values[i];
}

void DetectorId::setFieldValue(int i, FieldValue val) {
    values[i] = val;
}

void DetectorId::setFieldValue(const std::string& fieldName, FieldValue fieldValue) {
    values[idFieldMap[fieldName]->getIndex()] = fieldValue;
}

IdField::IdFieldList* DetectorId::getFieldList() {
    return idFieldList;
}

IdField* DetectorId::getField(const std::string& fieldName) {
    return idFieldMap[fieldName];
}

DetectorId::FieldValue DetectorId::getFieldValue(const std::string& fieldName) {
    return values[idFieldMap[fieldName]->getIndex()];
}
