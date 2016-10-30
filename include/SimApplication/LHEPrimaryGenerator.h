#ifndef SIMAPPLICATION_LHEPRIMARYGENERATOR_H_
#define SIMAPPLICATION_LHEPRIMARYGENERATOR_H_

// Geant4
#include "G4VPrimaryGenerator.hh"

// LDMX
#include "SimApplication/LHEReader.h"

namespace sim {

class LHEPrimaryGenerator : public G4VPrimaryGenerator {

    public:

        LHEPrimaryGenerator(LHEReader*);

        virtual ~LHEPrimaryGenerator();

        void GeneratePrimaryVertex(G4Event*);

    private:

        LHEReader* reader;
};

}

#endif
