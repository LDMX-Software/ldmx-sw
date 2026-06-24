/**
 * @file MagneticFieldStore.h
 * @brief Class providing a global store to access magnetic field objects
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 */

#ifndef SIMCORE_MAGNETICFIELDSTORE_H_
#define SIMCORE_MAGNETICFIELDSTORE_H_

// Geant4
#include <map>

#include "G4MagneticField.hh"

namespace simcore {

/**
 * @class MagneticFieldStore
 * @brief Global store to access magnetic field objects
 */
class MagneticFieldStore {
 public:
  /**
   * Map of names to magnetic fields.
   */
  using MagFieldMap = std::map<std::string, G4MagneticField*>;

  /**
   * Get the global instance of the magnetic field store.
   * @return The magnetic field store.
   */
  static MagneticFieldStore* getInstance() {
    static MagneticFieldStore instance;
    return &instance;
  }

  /**
   * Destructor
   *
   * Cleans up all stored G4MagneticFields
   */
  ~MagneticFieldStore() {
    for (auto& name_field : mag_fields_) {
      delete name_field.second;
    }
    mag_fields_.clear();
  }

  /**
   * Get a magnetic field by name.
   * @param name The name of the magnetic field.
   */
  G4MagneticField* getMagneticField(const std::string& name) {
    return mag_fields_.at(name);
  }

  /**
   * Add a magnetic field by name.
   * @param name The name of the magnetic field.
   * @param magField The magnetic field definition.
   */
  void addMagneticField(const std::string& name, G4MagneticField* magField) {
    mag_fields_[name] = magField;
  }

 private:
  /**
   * Map of names to magnetic fields.
   */
  MagFieldMap mag_fields_;
};

}  // namespace simcore

#endif
