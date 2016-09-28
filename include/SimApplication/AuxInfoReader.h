#ifndef SIMAPPLICATION_AUXINFOREADER_H_
#define SIMAPPLICATION_AUXINFOREADER_H_ 1

// Geant4
#include "G4GDMLParser.hh"

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

    private:
        G4GDMLParser* parser;
        G4GDMLEvaluator* eval;
};

#endif
