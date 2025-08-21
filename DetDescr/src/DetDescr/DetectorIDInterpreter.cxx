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
    DetectorIDInterpreter::g_rosetta_stone;

DetectorIDInterpreter::~DetectorIDInterpreter() {}
DetectorIDInterpreter::DetectorIDInterpreter() : id_(), p_field_info_(0) {
  init();
}

DetectorIDInterpreter::DetectorIDInterpreter(DetectorID did)
    : id_(did), p_field_info_(0) {
  init();
  unpack();
}

void DetectorIDInterpreter::setRawValue(DetectorID rawValue) {
  id_ = rawValue;
  init();
  unpack();
}

void DetectorIDInterpreter::unpack() {
  std::fill(field_values_.begin(), field_values_.end(), 0);
  if (!p_field_info_) return;
  for (auto field : p_field_info_->field_list_) {
    unsigned result = (field->getBitMask() & id_.raw()) >> field->getStartBit();
    this->field_values_[field->getIndex()] = result;
  }
}

void DetectorIDInterpreter::pack() {
  DetectorID::RawValue raw_value = 0;
  for (auto field : p_field_info_->field_list_) {
    unsigned field_value = field_values_[field->getIndex()];
    raw_value =
        raw_value | ((field_value << field->getStartBit()) & field->getBitMask());
  }
  id_.setRawValue(raw_value);
}

DetectorIDInterpreter::FieldValue DetectorIDInterpreter::getFieldValue(
    int i) const {
  IDField* field = p_field_info_->field_list_.at(i);
  unsigned result = (field->getBitMask() & id_.raw()) >> field->getStartBit();
  return result;
}

void DetectorIDInterpreter::setFieldValue(int i, FieldValue val) {
  field_values_[i] = val;
  pack();  // keep packed
}

void DetectorIDInterpreter::setFieldValue(const std::string& fieldName,
                                          FieldValue fieldValue) {
  auto byname = p_field_info_->field_map_.find(fieldName);
  if (byname != p_field_info_->field_map_.end())
    fieldValue[byname->second->getIndex()] = fieldValue;
  pack();  // keep packed
}

const IDField* DetectorIDInterpreter::getField(
    const std::string& fieldName) const {
  auto byname = p_field_info_->field_map_.find(fieldName);
  if (byname != p_field_info_->field_map_.end()) return (byname->second);
  return 0;
}

DetectorIDInterpreter::FieldValue DetectorIDInterpreter::getFieldValue(
    const std::string& fieldName) const {
  auto byname = p_field_info_->field_map_.find(fieldName);
  return getFieldValue(byname->second->getIndex());
}

void DetectorIDInterpreter::init() {
  if (g_rosetta_stone.empty()) loadStandardInterpreters();

  p_field_info_ = 0;

  if (id_.null()) return;

  for (auto ptr : g_rosetta_stone) {
    if ((id_.raw() & ptr.first.mask_) == ptr.first.comparison_) {
      p_field_info_ = (ptr.second);
      this->field_values_.resize(p_field_info_->field_list_.size());
      return;
    }
  }

  // fell through, no match
  IDSignature sig;
  sig.comparison_ = 0;
  sig.mask_ = DetectorID::SUBDETECTORID_MASK << DetectorID::SUBDETECTORID_SHIFT;

  auto ptr = g_rosetta_stone.find(sig);
  p_field_info_ = (ptr->second);
  this->field_values_.resize(p_field_info_->field_list_.size());
}

void DetectorIDInterpreter::registerInterpreter(
    SubdetectorIDType idtype, const IDField::IDFieldList& fieldList) {
  IDSignature sig;
  sig.comparison_ = idtype << DetectorID::SUBDETECTORID_SHIFT;
  sig.mask_ = DetectorID::SUBDETECTORID_MASK << DetectorID::SUBDETECTORID_SHIFT;
  if (g_rosetta_stone.find(sig) != g_rosetta_stone.end()) {
    EXCEPTION_RAISE("DetectorIDException",
                    "Attempted to replace interpreter for subdetector " +
                        std::to_string(idtype));
  }
  SubdetectorIDFields* fields = new SubdetectorIDFields();
  fields->field_list_ = fieldList;
  for (auto it : fieldList) fields->field_map_[it->getFieldName()] = it;
  g_rosetta_stone[sig] = fields;
}

void DetectorIDInterpreter::registerInterpreter(
    SubdetectorIDType idtype, unsigned int mask, unsigned int equality,
    const IDField::IDFieldList& fieldList) {
  IDSignature sig;
  sig.comparison_ = (idtype << DetectorID::SUBDETECTORID_SHIFT) | equality;
  sig.mask_ =
      (DetectorID::SUBDETECTORID_MASK << DetectorID::SUBDETECTORID_SHIFT) |
      mask;
  if (g_rosetta_stone.find(sig) != g_rosetta_stone.end()) {
    EXCEPTION_RAISE("DetectorIDException",
                    "Attempted to replace interpreter for subdetector " +
                        std::to_string(idtype) + " mask " +
                        std::to_string(mask) + " equality " +
                        std::to_string(equality));
  }
  SubdetectorIDFields* fields = new SubdetectorIDFields();
  fields->field_list_ = fieldList;
  for (auto it : fieldList) fields->field_map_[it->getFieldName()] = it;
  g_rosetta_stone[sig] = fields;
}

void DetectorIDInterpreter::loadStandardInterpreters() {
  if (!g_rosetta_stone.empty()) return;
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
