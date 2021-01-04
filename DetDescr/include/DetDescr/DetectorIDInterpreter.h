#ifndef DETDESCR_DETECTORIDINTERPRETER_H
#define DETDESCR_DETECTORIDINTERPRETER_H

// STL
#include <iostream>
#include <vector>

// LDMX
#include "DetDescr/DetectorID.h"
#include "DetDescr/IDField.h"

namespace ldmx {

/**
 * @class DetectorIDInterpreter
 * @brief Class provides an "introspection" capability for the 32-bit packed IDs
 * used for uniquely identifying hits and detector components
 *
 * @note
 * Represents an ID in the detector with a raw, 32-bit value which can
 * be unpacked into a list of field values or packed from a list of field
 * values.
 *
 */
class DetectorIDInterpreter {
 public:
  DetectorIDInterpreter();
  DetectorIDInterpreter(DetectorID detid);

  ~DetectorIDInterpreter();

  /**
   * Definition of the field value type.
   */
  typedef unsigned FieldValue;

  /**
   * A list of field values.
   */
  typedef std::vector<FieldValue> FieldValueList;

  /**
   * Get the raw value of the detector ID.
   * @return The raw value.
   */
  DetectorID getId() const { return id_; }

  /**
   * Set the raw value of the detector ID.
   * @param rawValue The raw value of the ID.
   */
  void setRawValue(DetectorID rawValue);

  /**
   * Decode and return a field's value from the raw ID.
   * @param i The index of the field value.
   */
  FieldValue getFieldValue(int i) const;

  /**
   * Set a field value by index in the field value list.
   * @param i The index of the field value.
   * @param value The new field value.
   */
  void setFieldValue(int i, FieldValue value);

  /**
   * Set a field value by its name.
   * @param fieldName The name of the field.
   * @param fieldValue The new value of the field.
   */
  void setFieldValue(const std::string& fieldName, FieldValue fieldValue);

  /**
   * Get the list of field information.
   * @return The list of field information.
   */
  int getFieldCount() const { return int(p_fieldInfo_->fieldList_.size()); }

  /**
   * Get the list of field information.
   * @return The list of field information.
   */
  const IDField::IDFieldList& getFieldList() const {
    return p_fieldInfo_->fieldList_;
  }

  /**
   * Get the information for a field by name.
   * @return The information of the field.
   *
   */
  const IDField* getField(const std::string& fieldName) const;

  /**
   * Decode and return a field's value by name (e.g. "layer").
   * @return The value of the field.
   */
  FieldValue getFieldValue(const std::string& fieldName) const;

  /**
   * Register a new field interpreter for a given subdetector id
   */
  static void registerInterpreter(SubdetectorIDType idtype,
                                  const IDField::IDFieldList& fieldList);

  /**
   * Register a new field interpreter for a more-complex case where additional
   * bits are needed to determine format
   */
  static void registerInterpreter(SubdetectorIDType idtype, unsigned int mask,
                                  unsigned int equality,
                                  const IDField::IDFieldList& fieldList);

 private:
  /**
   * Load the standard field interpreters if not yet loaded.
   * @important Developers of new Ids should add construction calls here!
   */
  static void loadStandardInterpreters();

  /**
   * Reinitialize the ID in case the field list changed.
   * This is called automatically by <i>setFieldList</i>.
   * Function should be called by a subclass if a new field list
   * is set outside that method or if new fields are added
   * to the existing list.
   */
  void init();

  /**
   * Unpack the current raw value into a list.
   */
  void unpack();

  /**
   * Pack the current list of field values into a raw value.
   */
  void pack();

  /**
   * The raw, packed value of the ID.
   */
  DetectorID id_;

  /**
   * The unpacked list of field values.
   */
  FieldValueList fieldValues_;

  struct SubdetectorIDFields {
    IDField::IDFieldMap fieldMap_;
    IDField::IDFieldList fieldList_;
  };

  struct IDSignature {
    unsigned int mask_;
    unsigned int comparison_;
    bool operator<(const IDSignature& id) const {
      return (comparison_ & mask_) < (id.comparison_ & id.mask_);
    }
  };

  static std::map<IDSignature, const SubdetectorIDFields*> g_rosettaStone;

  /**
   * Pointer to the appropriate field info for this class
   */
  const SubdetectorIDFields* p_fieldInfo_;
};

}  // namespace ldmx

#endif
