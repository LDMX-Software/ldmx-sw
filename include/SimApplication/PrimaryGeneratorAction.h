#ifndef SIMAPPLICATION_PRIMARYGENERATORACTION_H_
#define SIMAPPLICATION_PRIMARYGENERATORACTION_H_

// Geant4
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4VPrimaryGenerator.hh"

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {

    public:

        PrimaryGeneratorAction();

        virtual ~PrimaryGeneratorAction();

        virtual void GeneratePrimaries(G4Event* anEvent);

        void setPrimaryGenerator(G4VPrimaryGenerator*);

    private:
        G4VPrimaryGenerator* generator;
};

#endif
