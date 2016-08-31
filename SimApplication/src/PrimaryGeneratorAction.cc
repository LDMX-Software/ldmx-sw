#include "SimApplication/PrimaryGeneratorAction.h"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
 : G4VUserPrimaryGeneratorAction(), _particle_gun(0) {
    G4int n_particle = 1;
    _particle_gun = new G4ParticleGun(n_particle);

    G4ParticleTable* particle_table = G4ParticleTable::GetParticleTable();
    G4String particle_name;
    _particle_gun->SetParticleDefinition(particle_table->FindParticle(particle_name="geantino"));
    _particle_gun->SetParticleEnergy(1.0*GeV);
    _particle_gun->SetParticlePosition(G4ThreeVector(-2.0*m, 0.1, 0.1));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete _particle_gun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    G4int i = event->GetEventID() % 3;
    G4ThreeVector v(0.0,0.0,1.0);
    switch(i) {
        case 0:
            break;
        case 1:
            v.setY(0.1);
            break;
        case 2:
            v.setZ(0.1);
            break;
    }
    _particle_gun->SetParticleMomentumDirection(v);
    _particle_gun->GeneratePrimaryVertex(event);
}
