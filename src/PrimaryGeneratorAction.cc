#include "SimApplication/PrimaryGeneratorAction.h"

// Geant4
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction()
 : G4VUserPrimaryGeneratorAction(), generator(new G4ParticleGun) {
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete generator;
}

void PrimaryGeneratorAction::setPrimaryGenerator(G4VPrimaryGenerator* aGenerator) {
    PrimaryGeneratorAction::generator = aGenerator;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    std::cout << ">>> Begin Event " << event->GetEventID() << " <<<" << std::endl;
    generator->GeneratePrimaryVertex(event);
}
