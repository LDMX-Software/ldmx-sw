#include "SimApplication/SimParticleBuilder.h"

// LDMX
#include "SimApplication/UserTrackInformation.h"
#include "Event/RootEventWriter.h"

// Geant4
#include "G4SystemOfUnits.hh"

SimParticleBuilder::SimParticleBuilder() {
}

SimParticleBuilder::~SimParticleBuilder() {
}

void SimParticleBuilder::buildSimParticles() {

    particleMap.clear();

    TrackSummary::TrackSummaryList* trackList =
            TrackSummary::getTrackSummaryList();

    std::cout << "Track summary list has " << trackList->size() << " objects" << std::endl;

    RootEventWriter* writer = RootEventWriter::getInstance();
    Event* event = writer->getEvent();

    for (TrackSummary::TrackSummaryList::iterator it = trackList->begin();
            it != trackList->end(); it++) {
        SimParticle* p = (SimParticle*) event->addObject(Event::SIM_PARTICLES);
        TrackSummary* trackSummary = *it;
        std::cout << "Building new SimParticle for track ID " << trackSummary->getTrackID() << " ... " << std::endl;
        buildSimParticle(p, trackSummary);
        std::cout << "Created new SimParticle ..." << std::endl;
        p->Print(NULL);
        std::cout << std::endl;
        particleMap[trackSummary->getTrackID()] = p;
    }
}

SimParticle* SimParticleBuilder::buildSimParticle(SimParticle* p, TrackSummary* info) {

    p->setGenStatus(info->getGenStatus());
    p->setSimStatus(info->getSimStatus());
    p->setPdg(info->getPDG());
    p->setCharge(info->getCharge());
    p->setMass(info->getMass() / GeV);
    p->setEnergy(info->getEnergy());

    const G4ThreeVector& endPoint = info->getEndPoint();
    p->setEndPoint(endPoint[0], endPoint[1], endPoint[2]);

    const G4ThreeVector& momentum = info->getMomentum();
    p->setMomentum(momentum[0], momentum[1], momentum[2]);

    const G4ThreeVector& vertex = info->getVertex();
    p->setVertex(vertex[0], vertex[1], vertex[2]);

    p->setTime(info->getGlobalTime());

    return p;
}
