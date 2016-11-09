#ifndef SIMAPPLICATION_AUXINFOREADER_H_
#define SIMAPPLICATION_AUXINFOREADER_H_

// Geant4
#include "G4GDMLParser.hh"

namespace sim {

class AuxInfoReader {

    public:

        AuxInfoReader(G4GDMLParser*);

        virtual ~AuxInfoReader();

        void readGlobalAuxInfo();

        void assignAuxInfoToVolumes();

    private:

        void createSensitiveDetector(G4String sdType, const G4GDMLAuxListType* auxInfoList);

        void createDetectorID(G4String name, const G4GDMLAuxListType* auxInfoList);

        void createMagneticField(G4String name, const G4GDMLAuxListType* auxInfoList);

        void createRegion(G4String name, const G4GDMLAuxListType* auxInfoList);

        void createVisAttributes(G4String name, const G4GDMLAuxListType* auxInfoList);

    private:
        G4GDMLParser* parser_;
        G4GDMLEvaluator* eval_;
};

}

#endif
