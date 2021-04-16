#include "DetDescr/DetectorIDInterpreter.h"
#include "DetDescr/EcalID.h"
#include "DetDescr/EcalTriggerID.h"
#include "DetDescr/HcalID.h"
#include "DetDescr/SimSpecialID.h"
#include "DetDescr/TrackerID.h"
#include "DetDescr/TrigScintID.h"

namespace ldmx {

std::map<DetectorIDInterpreter::IDSignature,
         const DetectorIDInterpreter::SubdetectorIDFields*>
    DetectorIDInterpreter::g_rosettaStone;

DetectorIDInterpreter::~DetectorIDInterpreter() {}
DetectorIDInterpreter::DetectorIDInterpreter() : id_(), p_fieldInfo_(0) {
  init();
}

DetectorIDInterpreter::DetectorIDInterpreter(DetectorID did)
    : id_(did), p_fieldInfo_(0) {
  init();
  unpack();
}

void DetectorIDInterpreter::setRawValue(DetectorID rawValue) {
  id_ = rawValue;
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
  DetectorID::RawValue rawValue = 0;
  for (auto field : p_fieldInfo_->fieldList_) {
    unsigned fieldValue = fieldValues_[field->getIndex()];
    rawValue =
        rawValue | ((fieldValue << field->getStartBit()) & field->getBitMask());
  }
  id_.setRawValue(rawValue);
}

DetectorIDInterpreter::FieldValue DetectorIDInterpreter::getFieldValue(
    int i) const {
  IDField* field = p_fieldInfo_->fieldList_.at(i);
  unsigned result = (field->getBitMask() & id_.raw()) >> field->getStartBit();
  return result;
}

void DetectorIDInterpreter::setFieldValue(int i, FieldValue val) {
  fieldValues_[i] = val;
  pack();  // keep packed
}

void DetectorIDInterpreter::setFieldValue(const std::string& fieldName,
                                          FieldValue fieldValue) {
  auto byname = p_fieldInfo_->fieldMap_.find(fieldName);
  if (byname != p_fieldInfo_->fieldMap_.end())
    fieldValues_[byname->second->getIndex()] = fieldValue;
  pack();  // keep packed
}

const IDField* DetectorIDInterpreter::getField(
    const std::string& fieldName) const {
  auto byname = p_fieldInfo_->fieldMap_.find(fieldName);
  if (byname != p_fieldInfo_->fieldMap_.end()) return (byname->second);
  return 0;
}

DetectorIDInterpreter::FieldValue DetectorIDInterpreter::getFieldValue(
    const std::string& fieldName) const {
  auto byname = p_fieldInfo_->fieldMap_.find(fieldName);
  return getFieldValue(byname->second->getIndex());
}

void DetectorIDInterpreter::init() {
  if (g_rosettaStone.empty()) loadStandardInterpreters();

  p_fieldInfo_ = 0;

  if (id_.null()) return;

  for (auto ptr : g_rosettaStone) {
    if ((id_.raw() & ptr.first.mask_) == ptr.first.comparison_) {
      p_fieldInfo_ = (ptr.second);
      this->fieldValues_.resize(p_fieldInfo_->fieldList_.size());
      return;
    }
  }

  // fell through, no match
  IDSignature sig;
  sig.comparison_ = 0;
  sig.mask_ = DetectorID::SUBDETECTORID_MASK << DetectorID::SUBDETECTORID_SHIFT;

  auto ptr = g_rosettaStone.find(sig);
  p_fieldInfo_ = (ptr->second);
  this->fieldValues_.resize(p_fieldInfo_->fieldList_.size());
}

void DetectorIDInterpreter::registerInterpreter(
    SubdetectorIDType idtype, const IDField::IDFieldList& fieldList) {
  IDSignature sig;
  sig.comparison_ = idtype << DetectorID::SUBDETECTORID_SHIFT;
  sig.mask_ = DetectorID::SUBDETECTORID_MASK << DetectorID::SUBDETECTORID_SHIFT;
  if (g_rosettaStone.find(sig) != g_rosettaStone.end()) {
    EXCEPTION_RAISE("DetectorIDException",
                    "Attempted to replace interpreter for subdetector " +
                        std::to_string(idtype));
  }
  SubdetectorIDFields* fields = new SubdetectorIDFields();
  fields->fieldList_ = fieldList;
  for (auto it : fieldList) fields->fieldMap_[it->getFieldName()] = it;
  g_rosettaStone[sig] = fields;
}

void DetectorIDInterpreter::registerInterpreter(
    SubdetectorIDType idtype, unsigned int mask, unsigned int equality,
    const IDField::IDFieldList& fieldList) {
  IDSignature sig;
  sig.comparison_ = (idtype << DetectorID::SUBDETECTORID_SHIFT) | equality;
  sig.mask_ =
      (DetectorID::SUBDETECTORID_MASK << DetectorID::SUBDETECTORID_SHIFT) |
      mask;
  if (g_rosettaStone.find(sig) != g_rosettaStone.end()) {
    EXCEPTION_RAISE("DetectorIDException",
                    "Attempted to replace interpreter for subdetector " +
                        std::to_string(idtype) + " mask " +
                        std::to_string(mask) + " equality " +
                        std::to_string(equality));
  }
  SubdetectorIDFields* fields = new SubdetectorIDFields();
  fields->fieldList_ = fieldList;
  for (auto it : fieldList) fields->fieldMap_[it->getFieldName()] = it;
  g_rosettaStone[sig] = fields;
}

void DetectorIDInterpreter::loadStandardInterpreters() {
  if (!g_rosettaStone.empty()) return;
  IDField::IDFieldList fields;
  fields.push_back(
      new IDField("subdetector", 0, DetectorID::SUBDETECTORID_SHIFT, 31));
  fields.push_back(
      new IDField("payload", 2, 0, DetectorID::SUBDETECTORID_SHIFT - 1));

  registerInterpreter(SD_NULL, fields);

  EcalID::createInterpreters();
  EcalTriggerID::createInterpreters();
  HcalID::createInterpreters();
  TrackerID::createInterpreters();
  TrigScintID::createInterpreters();
  SimSpecialID::createInterpreters();
}
}  // namespace ldmx
