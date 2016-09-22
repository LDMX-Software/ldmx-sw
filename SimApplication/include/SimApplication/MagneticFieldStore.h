#ifndef SIMAPPLICATION_MAGFIELDSTORE_H_
#define SIMAPPLICATION_MAGFIELDSTORE_H_ 1

// Geant4
#include "G4MagneticField.hh"

class MagneticFieldStore {

    public:

        typedef std::map<std::string, G4MagneticField*> MagFieldMap;

        static MagneticFieldStore* getInstance() {
            static MagneticFieldStore INSTANCE;
            return &INSTANCE;
        }

        G4MagneticField* getMagneticField(const std::string& name) {
            return magFields[name];
        }

        void addMagneticField(const std::string& name, G4MagneticField* magField) {
            magFields[name] = magField;
        }

    private:

        MagFieldMap magFields;
};

#endif
