#include "SimApplication/LHEPrimaryGenerator.h"

// Geant4
#include "G4Event.hh"

LHEPrimaryGenerator::LHEPrimaryGenerator(LHEReader* theReader)
    : reader(theReader) {
}

LHEPrimaryGenerator::~LHEPrimaryGenerator() {
    delete reader;
}

/*
 * TODO: Set mother-dau pointers using G4Primary::SetDaughter method.
 * TODO: Set HEPEVT status using a G4VPrimaryParticleInformation class.
 */
void LHEPrimaryGenerator::GeneratePrimaryVertex(G4Event* anEvent) {

    std::cout << "Reading next LHE event ..." << std::endl;

    LHEEvent* lheEvent = reader->readNextEvent();

    if (lheEvent != NULL) {

        G4PrimaryVertex* vertex = new G4PrimaryVertex();
        vertex->SetPosition(0, 0, 0);
        vertex->SetWeight(lheEvent->getXWGTUP());

        std::vector<G4PrimaryParticle*> primaries;

        const std::vector<LHEParticle*>& particles = lheEvent->getParticles();
        for (std::vector<LHEParticle*>::const_iterator it = particles.begin();
                it != particles.end(); it++) {

            LHEParticle* particle = (*it);

            G4PrimaryParticle* primary = new G4PrimaryParticle();
            primary->SetPDGcode(particle->getIDUP());
            primary->SetMomentum(particle->getPUP(0), particle->getPUP(1), particle->getPUP(2));
            primary->SetTotalEnergy(particle->getPUP(3));
            primary->SetMass(particle->getPUP(4));
            primary->SetProperTime(particle->getVTIMUP());

            vertex->SetPrimary(primary);

            primaries.push_back(primary);

            std::cout << "Added next primary ..." << std::endl;
            std::cout << "  PDG: " << primary->GetPDGcode()
                    << "  momentum: " << primary->GetMomentum()
                    << ", energy: " << primary->GetTotalEnergy()
                    << ", mass: " << primary->GetMass()
                    << ", time: " << primary->GetProperTime()
                    << std::endl;
        }

        anEvent->AddPrimaryVertex(vertex);

    } else {
        G4Exception("LHEPrimaryGenerator::GeneratePrimaryVertex",
                "LHEEventUnderflow",
                RunMustBeAborted,
                "No more LHE events found in input file.");
    }
}
