#ifndef SIMAPPLICATION_PRIMARYGENERATORACTION_H_
#define SIMAPPLICATION_PRIMARYGENERATORACTION_H_

#include "G4VUserPrimaryGeneratorAction.hh"

#include "globals.hh"

class G4Event;
class G4ParticleGun;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {

    public:
        PrimaryGeneratorAction();
        ~PrimaryGeneratorAction();

       virtual void GeneratePrimaries(G4Event* anEvent);

    private:
        G4ParticleGun* _particle_gun;
};

#endif
