#include "SimApplication/PrimaryGeneratorAction.h"

// Geant4
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"

namespace sim {

PrimaryGeneratorAction::PrimaryGeneratorAction()
 : G4VUserPrimaryGeneratorAction(), generator_(new G4ParticleGun) {
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
    delete generator_;
}

void PrimaryGeneratorAction::setPrimaryGenerator(G4VPrimaryGenerator* aGenerator) {
    PrimaryGeneratorAction::generator_ = aGenerator;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    generator_->GeneratePrimaryVertex(event);
 
    // Activate the plugin manager hook.		
    pluginManager_->generatePrimary(event);
}

}
