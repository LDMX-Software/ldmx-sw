#ifndef SIMAPPLICATION_VISATTRIBUTESSTORE_H_
#define SIMAPPLICATION_VISATTRIBUTESSTORE_H_

// Geant4
#include "G4VisAttributes.hh"

namespace sim {

class VisAttributesStore {

    public:

        typedef std::map<std::string, G4VisAttributes*> VisAttributesMap;

        static VisAttributesStore* getInstance() {
            static VisAttributesStore INSTANCE;
            return &INSTANCE;
        }

        G4VisAttributes* getVisAttributes(const std::string& name) {
            return visAttributesMap_[name];
        }

        void addVisAttributes(const std::string& name, G4VisAttributes* visAttributes) {
            visAttributesMap_[name] = visAttributes;
        }

    private:

        VisAttributesMap visAttributesMap_;
};

}

#endif
