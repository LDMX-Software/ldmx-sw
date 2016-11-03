#include "SimApplication/PrimaryGeneratorAction.h"

// Geant4
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

namespace sim {

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

    //std::cout << "Dumping primary particles" << std::endl;
    //event->GetPrimaryVertex(0)->GetNumberOfParticle()
    //event->GetPrimaryVertex(0)->GetPrimary(0)->Print();
}

}
