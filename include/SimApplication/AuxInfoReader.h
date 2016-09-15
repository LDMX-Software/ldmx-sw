#ifndef SIMAPPLICATION_AUXINFOREADER_H_
#define SIMAPPLICATION_AUXINFOREADER_H_ 1

// Geant4
#include "G4GDMLParser.hh"

class AuxInfoReader {

public:

    AuxInfoReader(G4GDMLParser* theParser);

    void readGlobalAuxInfo();

    void assignSensDetsToVols();

private:
    void createSensitiveDetector(G4String sdType, const G4GDMLAuxListType* auxInfoList);

private:
    G4GDMLParser* parser;

};

#endif
