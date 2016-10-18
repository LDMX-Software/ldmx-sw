#ifndef SimApplication_PrimaryGeneratorAction_h
#define SimApplication_PrimaryGeneratorAction_h

// Geant4
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4VPrimaryGenerator.hh"

namespace sim {

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {

    public:

        PrimaryGeneratorAction();

        virtual ~PrimaryGeneratorAction();

        virtual void GeneratePrimaries(G4Event* anEvent);

        void setPrimaryGenerator(G4VPrimaryGenerator*);

    private:
        G4VPrimaryGenerator* generator;
};

}

#endif
