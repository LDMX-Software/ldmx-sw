#include "SimApplication/RootPrimaryGenerator.h"

// Geant4
#include "G4Event.hh"
#include "G4IonTable.hh"

// LDMX
#include "SimApplication/UserPrimaryParticleInformation.h"

// Geant4
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

namespace ldmx {

RootPrimaryGenerator::RootPrimaryGenerator( G4String filename ) {
    filename_ = filename;
}

RootPrimaryGenerator::~RootPrimaryGenerator() {
}

void RootPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

    std::cout << "Reading next Root event ..." << std::endl;

    // put in protection for if we run out of ROOT events

    G4PrimaryVertex* vertex = new G4PrimaryVertex();
    vertex->SetPosition(0, 0, 0);
    vertex->SetWeight(1.);

    G4PrimaryParticle* primary = new G4PrimaryParticle();
    primary->SetPDGcode(13);
    primary->SetMomentum(0. * MeV, 0. * MeV, 4000. * MeV);
    primary->SetMass(0.511 * MeV);
    //primary->SetProperTime(particle->getVTIMUP() * nanosecond);
    UserPrimaryParticleInformation* primaryInfo = new UserPrimaryParticleInformation();
    primaryInfo->setHepEvtStatus(1.);
    primary->SetUserInformation(primaryInfo);
    vertex->SetPrimary(primary);
    
    anEvent->AddPrimaryVertex(vertex);

}

}
